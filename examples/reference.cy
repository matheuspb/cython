def f(a&: int) -> void:
	a = 1

a = 0
f(a)
print(a)  # should print '1'
