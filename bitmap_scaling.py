bitmap = [
    1, 0,
    0, 1,
]

i = 0
for bit in bitmap:
    if i % 2 == 0:
        print()
    print(bit, end="")
    i += 1

print()
print("=====================")

width = 10
height = 10
scaled_bitmap = [9] * 16

i = 1
col = 0
row = 0
for bit in bitmap:
    scaled_bitmap[col+row] = bit
    scaled_bitmap[col+row+1] = bit
    scaled_bitmap[col+row+4] = bit
    scaled_bitmap[col+row+4+1] = bit
    col += 2
    if i == 2:
        i = 0
        col = 0
        row += 8
    i+=1

i = 0
for bit in scaled_bitmap:
    if i % 4 == 0:
        print()
    print(bit, end="")
    i+=1

