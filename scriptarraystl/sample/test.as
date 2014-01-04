void main()
{
	print("This is Angel Script\n");
	for(uint i = 0; i < string_array.length(); ++i)
	{
		print(string_array[i]);
	}

	string_array.resize(0);

	// now change it
	string_array.insertLast("\nHello");
	string_array.insertLast("\nTest Test");
	string_array.insertLast("\nThis is a long string. !This should not print.!");
	string_array[2].resize(23);

}