def factorial(n: int) -> int
	if n == 1 do
		return 1
	else
		return factorial(n-1) * factorial(n-2)
	end
end
