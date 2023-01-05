fmt:
	git ls-files | egrep '(\.h|\.cc)$$' | xargs -I {} clang-format -i -style=file {};
