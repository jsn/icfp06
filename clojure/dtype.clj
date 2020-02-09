(ns dtype
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

(defprotocol IVM (run [this out]))

(deftype VM
  [^int ^:unsynchronized-mutable pc
   ^"[Ljava.lang.Object;" ^:unsynchronized-mutable arrays
   ^long ^:unsynchronized-mutable narrays
   ^:unsynchronized-mutable free
   ^ints r]

  IVM

  (run [this out]
    (while (aget arrays 0)
      (let [ins (aget ^ints (aget arrays 0) pc)
            op (bit-and (bit-shift-right ins 28) 15)]
        (case op
          0 (when-not (zero? (C)) (A (B)))
          1 (A (aget ^ints (aget arrays (B)) (C)))
          2 (aset ^ints (aget arrays (A)) (B) (C))
          3 (A (unchecked-add-int (B) (C)))
          4 (A (unchecked-multiply-int (B) (C)))
          5 (A (Integer/divideUnsigned (B) (C)))
          6 (A (bit-not (bit-and (B) (C))))
          7 (do
              (aset arrays 0 nil)
              (binding [*out* *err*] (println "arrays:" narrays)))
          8 (let [a (int-array (C) 0)]
              (if-let [ai (peek free)]
                (do
                  (aset arrays ai a)
                  (set! free (pop free))
                  (B (int ai)))
                (let [len (alength arrays)]
                  (when (= narrays len)
                    (set! arrays (java.util.Arrays/copyOf arrays (* 2 len))))
                  (do
                    (B (int narrays))
                    (aset arrays narrays a)
                    (set! narrays (inc narrays))))))
          9 (do
              (set! free (conj free (C)))
              (aset arrays (C) nil))
          10 (.write ^java.io.OutputStream out (C))
          11 (C (.read ^java.io.Reader *in*))
          12 (do
               (when-not (zero? (B))
                 (let [zero (aclone ^ints (aget arrays (B)))]
                   (aset arrays 0 zero)))
               (set! pc (int (dec (C)))))
          13 (X (bit-and ins 0x1ffffff))
          (throw (ex-info "unknown opcode" {:opcode ins})))
        (set! pc (int (inc pc))
        ))))
  )

(defn code->VM ^VM [code]
  (let [arrays (make-array Object 32768)]
    (aset ^"[Ljava.lang.Object;" arrays 0 code)
    (->VM 0 arrays 1 '() (int-array 8 0))))

(defn run [code]
  (with-open [out (java.io.FileOutputStream. (java.io.FileDescriptor/out))]
    (-> code code->VM (.run out))))

(defn -main [& [file]]
  (run (ingest file)))
