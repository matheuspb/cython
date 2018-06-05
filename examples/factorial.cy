def factorial(n: int) -> int begin
	if n == 1 do
		return 1
	else
		return n * factorial(n-1)
	end
end
