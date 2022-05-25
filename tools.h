typedef unsigned long long ull;

int lpctlen(LPCTSTR string) {
	int len = 0;
	for (; string[len] != '\0'; len++);
	return len;
}

LPCTSTR lpctcat(LPCTSTR str1, LPCTSTR str2) {
	static LPCTSTR new_str;
	memmove(&new_str, str1, sizeof(str1));
	memmove(&new_str + lpctlen(new_str), str2, sizeof(str2));
	return new_str;
}