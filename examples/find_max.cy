def find_max(v: int[], size: int) -> int begin
	max = v[0]
	for i: int = 1; i < size; i = i + 1 begin
		if v[i] > max do
			max = v[i]
		end
	end
	return max
end
