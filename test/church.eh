#!/usr/bin/ehi

include '../lib/library.eh'

# Church numerals

decode f = (f (1.operator+)) 0
add m = n => f => x => (m f) ((n f) x)
multiply m = n => f => m (n f)
succ m = f => x => f ((m f) x)
exp m = n => n m

two = f => x => f (f x)
three = succ two
six = (multiply two) three
seven = (add ((add two) two)) three

answer = (multiply six) seven
sixtyfour = (exp two) six

echo (decode sixtyfour)
echo (decode answer)

# Church booleans
ctrue a = b => a
cfalse a = b => b
cdecode f = (f true) false

cand m = n => (m n) m
cor m = n => (m m) n
cnot m = (m cfalse) ctrue
cxor a = b => (a (cnot b)) b
cif m = a => b => (m a) b

echo (cdecode ((cand ctrue) cfalse))
echo (cdecode ((cor ctrue) cfalse))
echo (cdecode (cnot ctrue))

echo (decode (((cif ctrue) sixtyfour) answer))

# Church pairs
pair x = y => z => (z x) y
fst p = p (x => y => x)
snd p = p (x => y => y)
pdecode p = (fst p, snd p)

printvar (pdecode ((pair 2) 1))
echo (fst ((pair 2) 1))

# Church lists
nil = (pair ctrue) ctrue
isnil = fst
cons h = t => (pair cfalse) ((pair h) t)
head z = fst (snd z)
tail z = snd (snd z)
ldecode l = if cdecode (isnil l)
	Nil
else
	Cons(decode (head l), ldecode (tail l))
end

my_list = (cons two) ((cons three) nil)
echo (ldecode my_list)

