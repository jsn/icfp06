
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

private macro next_op(nxt)
  pc = {{nxt}}
  ins = zero[pc]
  case (ins >> 28) & 15
  {% for i in 0 .. 13 %}
  when {{i}}
    return op{{i}}.call(ins, pc)
  {% end %}
  else
    raise "unknown op"
  end
end

private macro def_op(n, nxt = pc + 1)
  op{{n}} = -> (ins : UInt32, pc : UInt32) {
    {{yield}}
    next_op {{nxt}}
  }
end

def main
  arrays = [] of Array(UInt32)
  free = [] of UInt32
  r = Array(UInt32).new 8, 0_u32

  if ARGV.empty?
    STDERR.puts "usage: #{$0} <file.um>"
    exit -1
  end

  zero : Array(UInt32) = load_file ARGV[0]
  arrays << zero

  {% for i in 0 .. 13 %}
    op{{i}} = -> (ins : UInt32, pc : UInt32) {}
  {% end %}

  def_op 0 { r[a] = r[b] if r[c] != 0 }

  def_op 1 { r[a] = arrays[r[b]][r[c]] }

  def_op 2 { arrays[r[a]][r[b]] = r[c] }

  def_op 3 { r[a] = r[b] + r[c] }

  def_op 4 { r[a] = r[b] * r[c] }

  def_op 5 { r[a] = r[b] / r[c] }

  def_op 6 { r[a] = ~(r[b] & r[c]) }

  def_op 7 do
    puts "HALT"
    return
  end

  def_op 8 do
    ar = Array.new r[c], 0_u32
    if free.empty?
      arrays << ar
      r[b] = (arrays.size - 1).to_u32
    else
      i = free.pop
      arrays[i] = ar
      r[b] = i
    end
  end

  def_op 9 do
    arrays[r[c]] = NUL
    free.push r[c]
  end

  def_op 10 { STDOUT.print r[c].chr }

  def_op 11 do
    STDOUT.flush
    x = STDIN.read_byte
    r[c] = x ? x.to_u32 : ~0_u32
  end

  def_op 12, r[c] { arrays[0] = zero = arrays[r[b]].clone if r[b] != 0 }

  def_op 13 { r[a2] = ins & 0x1ffffff }

  next_op 0_u32
end

{% if ! flag? :release %}
  puts "This will not work without --release compile flag (no TCE). Exiting."
  exit -1
{% end %}

main
