s/\(+[0-9a-f][0-9a-f]*H\)/(\1)/g
s/\(-[0-9a-f][0-9a-f]*H\)/(\1)/g
s/repe    movsd/rep movsd/g
s/repe    stosd/rep stosd/g
s/fcompp  st(1)/fcompp/g
s/^                EXTRN/                EXTERNDEF/g
s/^                DP/                DF/g

