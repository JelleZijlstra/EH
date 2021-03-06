#!/usr/bin/ehi

# Church numerals

decode f = f (1.operator+) 0
add m n f x = m f (n f x)
multiply m n f = m (n f)
succ m f x = f (m f x)
pred n f x = n (g => h => h (g f)) (u => x) (u => u)
sub m n = (n pred) m
exp m n = n m
iszero n = n (_ => false) true

two f x = f (f x)
three = succ two
six = multiply two three
seven = add (add two two) three

answer = multiply six seven
sixtyfour = exp two six

echo (decode sixtyfour)
echo (decode answer)
echo (decode (pred sixtyfour))
echo (decode (sub sixtyfour answer))
echo (iszero sixtyfour)
echo (iszero (pred (pred two)))

# Church booleans
ctrue a b = a
cfalse a b = b
cdecode f = f true false

cand m n = m n m
cor m n = m m n
cnot m = m cfalse ctrue
cxor a b = a (cnot b) b
cif m a b = m a b

echo (cdecode (cand ctrue cfalse))
echo (cdecode (cor ctrue cfalse))
echo (cdecode (cnot ctrue))

echo (decode (cif ctrue sixtyfour answer))

# Church pairs
pair x y z = z x y
fst p = p ctrue
snd p = p cfalse
pdecode p = (fst p, snd p)

printvar (pdecode (pair 2 1))
echo (fst (pair 2 1))

# Church lists
nil = pair ctrue ctrue
isnil = fst
cons h t = pair cfalse (pair h t)
head z = fst (snd z)
tail z = snd (snd z)

include '../lib/library.eh'

ldecode l = if cdecode (isnil l)
	Nil
else
	Cons(decode (head l), ldecode (tail l))
end

my_list = cons two (cons three nil)
echo (ldecode my_list)

