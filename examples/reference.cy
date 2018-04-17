def f(a&: int) -> void begin
	a = 1
end

def main() -> void begin
	a: int = 0
	f(a)
	# test
	print(a)  # should print '1'
end
