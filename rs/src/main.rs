
use std::env ;
use std::fs ;
use std::io ;
use std::io::prelude::* ;
use std::collections::HashMap ;
use std::slice ;

fn read_program(fname: &str) -> std::io::Result<Vec<u32>> {
    let len : usize = fs::metadata(fname)?.len().try_into().unwrap() ;

    if len % 4 != 0 {
        panic!("bad file length") ;
    }

    let nwords = len / 4 ;
    let mut v : Vec<u32> = Vec::with_capacity(nwords) ;

    let mut reader = io::BufReader::new(fs::File::open(fname)?) ;
    let mut b : [u8; 4] = [0; 4] ;

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
    let mut arrs : HashMap<u32, Vec<u32>> = HashMap::new() ;

    let mut pc = 0_u32 ;
    let mut r = [0_u32; 8] ;
    let mut vcnt = 1 ;

    macro_rules! ARR {
        ($e:expr) => {
            {
                let a = $e ;
                (if a == 0 { &mut zero } else { arrs.get_mut(&a).unwrap() })
            }
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
            7 => break,
            8 => {
                vcnt += 1 ;
                arrs.insert(vcnt, vec![0_u32; R![C] as usize]) ;
                R![B] = vcnt ;
            }
            9 => { arrs.remove(&R![C]) ; }
            10 => {
                let b = R![C] as u8 ;
                io::stdout().write(slice::from_ref(&b))? ;
            }
            11 => {
                let mut b = 0_u8 ;
                io::stdin().read(slice::from_mut(&mut b))? ;
                R![C] = b as u32 ;
            }
            12 => {
                let b = R![B] ;
                if b != 0 {
                    zero = arrs.get(&b).unwrap().clone()
                }
                pc = R![C].wrapping_sub(1) ;
            }
            13 => R![X] = ins & 0x1ffffff,
            _ => panic!("bad op")
        }
        pc = pc.wrapping_add(1) ;
    }

    Ok(())
}
