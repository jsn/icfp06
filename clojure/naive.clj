(ns naive
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
        v))))

(def mk-array
 (memoize
  (fn [n] (->> 0 int (repeat n) vec))))

(defmacro defregister [sym shift]
  (let [i (if (zero? shift) 'ins `(bit-shift-right ~'ins ~shift))
        rf `(~'r (bit-and ~i 7))]
  `(defmacro ~sym
     ([] '(int (~@rf)))
     ([new] (concat '(assoc ~@rf) `(~new))))))

(defregister C 0)
(defregister B 3)
(defregister A 6)
(defregister X 25)

(defn run [code]
  (loop [pc 0
         arrays [code]
         free []
         r (mk-array 8)]
    (let [ins (get-in arrays [0 pc])
          op (bit-and (bit-shift-right ins 28) 15)]
      ; (printf "%3d %d %d %d %d %d\n" pc op (A) (B) (C) (X))
      (case op
        0 (recur (inc pc) arrays free (if (zero? (C)) r (A (B))))
        1 (recur (inc pc) arrays free (A ((arrays (B)) (C))))
        2 (recur (inc pc) (assoc-in arrays [(A) (B)] (C)) free r)
        3 (recur (inc pc) arrays free (A (unchecked-add-int (B) (C))))
        4 (recur (inc pc) arrays free (A (unchecked-multiply-int (B) (C))))
        5 (recur (inc pc) arrays free (A (Integer/divideUnsigned (B) (C))))
        6 (recur (inc pc) arrays free (A (int (bit-not (bit-and (B) (C))))))
        7 :halt
        8 (let [c (C)
                a (mk-array c)]
            (if-let [ai (peek free)]
              (recur (inc pc) (assoc arrays ai a) (pop free) (B ai))
              (recur (inc pc) (conj arrays a) free (B (int (count arrays))))))
        9 (let [arrays (assoc arrays (C) nil)]
            (recur (inc pc) arrays (conj free (C)) r))
        10 (let [c (char (C))]
             (print c)
             (when (= c \newline) (flush))
             (recur (inc pc) arrays free r))
        11 (do
             (flush)
             (let [c (.read ^java.io.Reader *in*)]
               (recur (inc pc) arrays free (C (int c)))))
        12 (let [arrays
                 (cond-> arrays (not (zero? (B))) (assoc 0 (arrays (B))))]
             (recur (C) arrays free r))
        13 (recur (inc pc) arrays free (X (int (bit-and ins 0x1ffffff))))
        (throw (ex-info "unknown opcode" {:opcode ins}))))
  ))

(defn -main [& [file]]
  (run (ingest file)))
