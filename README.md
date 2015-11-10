*varint* is a library for inverted index compression, using the `__builtin_ia32_pshufb128` SIMD instruction.

It effectively compresses sorted integer arrays and performs highly efficient operations on compressed arrays:
- Fast sequential iteration speed
- Fast random lookup of docIds
- Fast finding intersections. (lookup in combination of several bitmaps index)

Compression needs to be data-type and data-distribution dependent. Decompresses small vector into CPU cache (not RAM):
- Decoding loop over cache resident compressed vector
- Avoid control dependencies within decoding loop (Prevent loop-pipelining)
- Avoid data dependencies between loop iteration


## Build
To compile, run
```bash
g++ -std=c++11 -O2 -mssse3 ./tests/main.cpp
```

## References
* http://www.stepanovpapers.com/CIKM_2011.pdf 
* http://www.adms-conf.org/p1-SCHLEGEL.pdf
* http://pharm.ece.wisc.edu/papers/ispass2011-shi.pdf

## Performance data
```
Memcache RPC latency:
  Server takes 70 000 ns to turn around data
  Total latency 200 000 ns within a rack
  400 000 -500 000 ns across datacenter

l1 cache reference = 0.5 ns
Branch mispredict = 5ns
l2 cache reference = 7ns
mutex lock/unlock = 25 ns
Main memory reference = 100ns
Compress 1K w/cheap compression = 3,000 ns
Send 2K bytes over 1 Gbps network = 20,000 ns
Read 1MB sequentially from memory  = 250,000 ns
Round trip within same dataenter = 500,000 ns
Disk seek = 10,000,000 ns
Read 1 MB sequentially from disk = 20,000,000 ns
send packet CA->Netherlands->CA = 150,000,000 ns
```