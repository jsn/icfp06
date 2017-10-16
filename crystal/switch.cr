
NUL = [] of UInt32

def load_file(fname)
  st = File.stat fname
  raise "weird file size #{st.size}" if st.size % sizeof(UInt32) != 0
  File.open fname do |f|
    Array.new st.size / sizeof(UInt32) do |i|
      f.read_bytes UInt32, IO::ByteFormat::BigEndian
    end
  end
end

private macro c; ins & 7; end
private macro b; (ins >> 3) & 7; end
private macro a; (ins >> 6) & 7; end
private macro a2; (ins >> 25) & 7; end

def main
  arrays = [] of Array(UInt32)
  free = [] of UInt32

  if ARGV.empty?
    STDERR.puts "usage: #{$0} <file.um>"
    exit -1
  end

  zero = load_file ARGV[0]
  arrays << zero

  pc = 0_u32
  r = Array.new 8, 0_u32

  loop do
    ins = zero[pc]

    case (ins >> 28) & 15
    when 0
      r[a] = r[b] if r[c] != 0
    when 1
      r[a] = arrays[r[b]][r[c]]
    when 2
      arrays[r[a]][r[b]] = r[c]
    when 3
      r[a] = r[b] + r[c]
    when 4
      r[a] = r[b] * r[c]
    when 5
      r[a] = r[b] / r[c]
    when 6
      r[a] = ~(r[b] & r[c])
    when 7
      puts "HALT"
      break
    when 8
      ar = Array.new r[c], 0_u32
      if free.empty?
        arrays << ar
        r[b] = (arrays.size - 1).to_u32
      else
        i = free.pop
        arrays[i] = ar
        r[b] = i
      end
    when 9
      arrays[r[c]] = NUL
      free.push r[c]
    when 10
      STDOUT.print r[c].chr
    when 11
      STDOUT.flush
      x = STDIN.read_byte
      r[c] = x.is_a?(UInt8) ? x.to_u32 : ~0_u32
    when 12
      arrays[0] = zero = arrays[r[b]].clone if r[b] != 0
      pc = r[c] - 1
    when 13
      r[a2] = ins & 0x1ffffff
    when 14, 15
      raise "unknown opcode"
    end

    pc += 1
  end
end

main
