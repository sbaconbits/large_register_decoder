# large_register_decoder

I find this tool very useful when working with 32-bit registers, it's most useful for decoding segments of a given register, the output looks like this:

~~~~
$> ./lrd -c registers.cfg -r CE3CFG -d 0x12341234 -e 0,0,4,35,2,0,36,3,0,4
Decode reg CE3CFG with value 0x12341234
33222222222211111111110000000000    <--| Bit Positions
10987654321098765432109876543210    <--|
[------][------][------][------]    <---- Byte indicator
[--][--][--][--][--][--][--][--]    <---- Nibble indicator
00010010001101000001001000110100
0|----||||||---||||------|||-||| [   31] SS         0 : 0x0
 0----||||||---||||------|||-||| [   30] EW         0 : 0x0
  0100||||||---||||------|||-||| [29:26] W_SETUP    4 : 0x4
      100011---||||------|||-||| [25:20] W_STROBE  35 : 0x23
            010||||------|||-||| [19:17] W_HOLD     2 : 0x2
               0000------|||-||| [16:13] R_SETUP    0 : 0x0
                   100100|||-||| [12:07] R_STROBE  36 : 0x24
                         011-||| [06:04] R_HOLD     3 : 0x3
                            0||| [   03] TA         0 : 0x0
                             100 [02:00] ASIZE      4 : 0x4
Encoded value 0x12341234
~~~~

This example output uses the example register configuration file in this project (registers.cfg).
