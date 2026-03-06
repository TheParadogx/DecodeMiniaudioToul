#pragma once
#include "miniaudio.h"

#include<filesystem>
#include<string>

namespace fs = std::filesystem;

class AudioConverter
{
	/// <summary>
	/// エラーメッセージ表示
	/// </summary>
	static void print_impl_ma_error(const std::wstring& Message, ma_result Result);
public:
	/// <summary>
	/// 音声ファイルを.aud(バイナリ)に変換する
	/// </summary>
	/// <param name="InputPath">変換したいファイルパス</param>
	/// <param name="OverWrite">true:上書きする</param>
	/// <returns>true:成功</returns>
	static bool Convert(const fs::path& InputPath, bool  OverWrite = true);

};

