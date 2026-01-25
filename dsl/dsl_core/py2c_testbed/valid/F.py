# Mixed constructs

total: int = 0
for i in range(1, 4):
	total = total + i
	if total % 2 == 0:
		while True:
			if total < 0:
				total = total + 3
				continue
			total = total + 1
			if total < 10:
				break
