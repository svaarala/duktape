def test()
	i = 0
	while i < 1 do
		t = ''
		j = 0
		while j < 1e5 do
			t += 'x'
			#print(t.length)
			#print("\n")
			j += 1
		end
		i += 1
	end
end

test()
