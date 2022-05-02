
use std::env ;
use std::fs ;
use std::io ;
use std::io::prelude::* ;

fn read_program(fname: &str) -> std::io::Result<Vec<u32>> {
    let len : usize = fs::metadata(fname)?.len().try_into().unwrap() ;

    if len % 4 != 0 {
        panic!("bad file length") ;
    }

    let nwords = len / 4 ;
    let mut v = Vec::with_capacity(nwords) ;

    let mut reader = io::BufReader::new(fs::File::open(fname)?) ;
    let mut b = [0_u8; 4] ;

    for _ in 0..nwords {
        reader.read_exact(&mut b)? ;
        v.push(u32::from_be_bytes(b)) ;
    }
    Ok(v)
}

const C : usize = 0 ;
const B : usize = 3 ;
const A : usize = 6 ;
const X : usize = 25 ;

fn main() -> std::io::Result<()> {
    let args : Vec<String> = env::args().collect() ;
    let fname = args.get(1).expect("program file must be specified") ;
    let mut zero = read_program(fname)? ;
    let mut free = Vec::new() ;
    let mut arrs = vec![vec![0_u32]] ;

    let mut pc = 0_u32 ;
    let mut r = [0_u32; 8] ;

    macro_rules! ARR {
        ($e:expr) => {
            (if $e == 0 { &mut zero } else { &mut arrs[$e as usize] })
        }
    }

    loop {
        let ins = zero[pc as usize] ;
        macro_rules! R {
            ($e:expr) => { r[((ins >> $e) & 7) as usize] }
        }
        match (ins >> 28) & 15 {
            0 => if R![C] != 0 { R![A] = R![B] },
            1 => R![A] = ARR![R![B]][R![C] as usize],
            2 => ARR![R![A]][R![B] as usize] = R![C],
            3 => R![A] = R![B].wrapping_add(R![C]),
            4 => R![A] = R![B].wrapping_mul(R![C]),
            5 => R![A] = R![B].wrapping_div(R![C]),
            6 => R![A] = !(R![B] & R![C]),
            7 => {
                println!("arrs len {}, free len {}", arrs.len(), free.len()) ;
                break ;
            }
            8 => {
                let v = vec![0_u32; R![C] as usize] ;
                if let Some(i) = free.pop() {
                    R![B] = i ;
                    arrs[i as usize] = v ;
                } else {
                    R![B] = arrs.len() as u32 ;
                    arrs.push(v) ;
                }
            }
            9 => {
                free.push(R![C]) ;
                arrs[R![C] as usize].clear() ;
                arrs[R![C] as usize].shrink_to_fit() ;
            }
            10 => {
                io::stdout().write_all(&[R![C] as u8])? ;
            }
            11 => {
                let mut b = [0_u8] ;
                io::stdin().read_exact(&mut b)? ;
                R![C] = b[0] as u32 ;
            }
            12 => {
                if R![B] != 0 {
                    zero = arrs[R![B] as usize].clone()
                }
                pc = R![C].wrapping_sub(1) ;
            }
            13 => R![X] = ins & 0x1ffffff,

            bad => panic!("bad op {}", bad)
        }
        pc = pc.wrapping_add(1) ;
    }

    Ok(())
}
