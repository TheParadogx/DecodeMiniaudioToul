#include"AudioConverter.hpp"

#include<iostream>

#define NONE_ARGS_TEST _DEBUG
#if NONE_ARGS_TEST
int main()
#else
int main(int argc, char* argv[])
#endif // NONE_ARGS_TEST
{
#if NONE_ARGS_TEST
	//	テスト用のフォルダ
	fs::path InputPath = "Assets/test.mp3";

#else
	//	.exe だけなら
	if (argc < 2)
	{
		std::wcout << L"Usage: PlayAud.exe <input.aud>" << std::endl;
		return -1;
	}

	fs::path InputPath = argv[1];

#endif // NONE_ARGS_TEST

	if (fs::exists(InputPath) == false)
	{
		std::wcout << L"Input file not found" << std::endl;
		return -1;
	}

	//	フォルダ指定の場合は中のファイルを捜査。ファイル指定の場合は単体処理
	if (fs::is_directory(InputPath))
	{
		std::wcout << L"Processing directory: " << InputPath << std::endl;
		for (const auto& entry : fs::directory_iterator(InputPath))
		{
			if (entry.is_regular_file())
			{
				AudioConverter::Convert(entry.path());
			}
		}
	}
	else
	{
		AudioConverter::Convert(InputPath);
	}

	std::wcout << L"Success." << std::endl;
	return 0;
}