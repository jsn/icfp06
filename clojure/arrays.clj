(ns arrays
  (:require [clojure.test :refer [deftest testing is]]
            [clojure.java.io :as io]
            [org.clojars.smee.binary.core :as b])
  (:gen-class))

(set! *warn-on-reflection* true)
(set! *unchecked-math* true)

(def codec (b/compile-codec (b/repeated :int-be)))

(defn ingest [path]
  (let [f (io/file path)]
    (with-open [r (io/input-stream f)]
      (let [v (b/decode codec r)]
        (assert (= (* 4 (count v)) (.length f)) "weird file size")
        (int-array v)))))

(defmacro defregister [sym shift]
  (let [i (if (zero? shift) 'ins `(bit-shift-right ~'ins ~shift))
        rf `(bit-and ~i 7)]
  `(defmacro ~sym
     ([] '(aget ~(with-meta 'r {:tag ints}) ~rf))
     ([new] (concat '(aset ~'r ~rf) `(~new))))))

(defregister C 0)
(defregister B 3)
(defregister A 6)
(defregister X 25)

(defmacro with-next [& body] `(do ~@body (recur (inc ~'pc) ~'zero)))

(defn run [^ints code]
  (let [out (java.io.FileOutputStream. (java.io.FileDescriptor/out))
        arrays (transient [code])
        free (volatile! '())
        r (int-array 8 0)]
    (loop [pc (int 0) zero code]
      (let [ins (aget ^ints zero pc)
            op (bit-and (bit-shift-right ins 28) 15)]
        ; (printf "%3d %d %d %d %d %d\n" pc op (A) (B) (C) (X))
        (case op
          0 (with-next (if (zero? (C)) r (A (B))))
          1 (with-next (A (aget ^ints (arrays (B)) (C))))
          2 (with-next (aset ^ints (arrays (A)) (B) (C)))
          3 (with-next (A (unchecked-add-int (B) (C))))
          4 (with-next (A (unchecked-multiply-int (B) (C))))
          5 (with-next (A (Integer/divideUnsigned (B) (C))))
          6 (with-next (A (int (bit-not (bit-and (B) (C))))))
          7 (do
              (binding [*out* *err*] (println "arrays:" (count arrays)))
              :halt)
          8 (with-next
              (let [c (C)
                    a (int-array c 0)]
                (if-let [ai (peek @free)]
                  (do
                    (assoc! arrays ai a)
                    (vswap! free pop)
                    (B (int ai)))
                  (do
                    (B (count arrays))
                    (conj! arrays a)))))
          9 (with-next
              (vswap! free conj (C))
              (assoc! arrays (C) nil))
          10 (with-next
               (let [c (char (C))]
                 (.write out (C))
                 (when (= c \newline)
                   (flush))))
          11 (with-next
               (flush)
               (C (int (.read ^java.io.Reader *in*))))
          12 (if-not (zero? (B))
               (let [zero (aclone ^ints (arrays (B)))]
                 (assoc! arrays 0 zero)
                 (recur (C) zero))
               (recur (C) zero))
          13 (with-next (X (int (bit-and ins 0x1ffffff))))
          (throw (ex-info "unknown opcode" {:opcode ins}))))
  )))

(defn -main [& [file]]
  (run (ingest file)))
