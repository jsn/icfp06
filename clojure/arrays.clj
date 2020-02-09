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
       ([] '(aget ~'r ~rf))
       ([new] (concat '(aset ~'r ~rf) `(~new))))))

(defregister C 0)
(defregister B 3)
(defregister A 6)
(defregister X 25)

(defmacro with-next [& body]
  `(do ~@body (recur (inc ~'pc) ~'arrays)))

(defn run [^ints code]
  (with-open [out (java.io.FileOutputStream. (java.io.FileDescriptor/out))]
    (let [
          ^"[Ljava.lang.Object;" arrays (make-array Object 32768)
          narrays (volatile! 1)
          free (volatile! '())
          r (int-array 8 0)]
      (aset arrays 0 code)
      (loop [pc 0
             arrays arrays]
        (let [ins (aget ^ints (aget arrays 0) pc)
              op (bit-and (bit-shift-right ins 28) 15)]
          ; (printf "%3d %d %d %d %d %d\n" pc op (A) (B) (C) (X))
          (case op
            0 (with-next (when-not (zero? (C)) (A (B))))
            1 (with-next (A (aget ^ints (aget arrays (B)) (C))))
            2 (with-next (aset ^ints (aget arrays (A)) (B) (C)))
            3 (with-next (A (unchecked-add-int (B) (C))))
            4 (with-next (A (unchecked-multiply-int (B) (C))))
            5 (with-next (A (Integer/divideUnsigned (B) (C))))
            6 (with-next (A (bit-not (bit-and (B) (C)))))
            7 (do
                (binding [*out* *err*] (println "arrays:" @narrays))
                :halt)
            8 (let [a (int-array (C) 0)]
                (if-let [ai (peek @free)]
                  (with-next
                    (aset arrays ai a)
                    (vswap! free pop)
                    (B (int ai)))
                  (let [len (alength arrays)
                        arrays
                        (if-not (= @narrays len) arrays
                          (java.util.Arrays/copyOf arrays (* 2 len)))]
                    (with-next
                      (B (int @narrays))
                      (aset arrays @narrays a)
                      (vswap! narrays inc)))))
            9 (with-next
                (vswap! free conj (C))
                (aset arrays (C) nil))
            10 (with-next (.write out (C)))
            11 (with-next (C (.read ^java.io.Reader *in*)))
            12 (if-not (zero? (B))
                 (let [zero (aclone ^ints (aget arrays (B)))]
                   (aset arrays 0 zero)
                   (recur (C) arrays))
                 (recur (C) arrays))
            13 (with-next (X (bit-and ins 0x1ffffff)))
            (throw (ex-info "unknown opcode" {:opcode ins}))))
        ))))

(defn -main [& [file]]
  (run (ingest file)))
