#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "AudioConverter.hpp"
#include "AudioHeader.hpp"

#include<iostream>
#include<fstream>
#include<vector>

/// <summary>
/// エラーメッセージ表示
/// </summary>
void AudioConverter::print_impl_ma_error(const std::wstring& Message, ma_result Result)
{
	std::wcerr 
		<< L"[ERROR]:" << Message
		<< L"(Result: " << Result
		<< L" - " << ma_result_description(Result) << L")" << std::endl;
}

/// <summary>
/// 音声ファイルを.aud(バイナリ)に変換する
/// </summary>
/// <param name="InputPath">変換したいファイルパス</param>
/// <param name="OverWrite">true:上書きする</param>
/// <returns>true:成功</returns>
bool AudioConverter::Convert(const fs::path& InputPath, bool OverWrite)
{
	//	拡張子の判定
	static const std::vector<std::wstring> Extensions = { L".wav", L".mp3", L".flac", L".ogg" };
	bool Supported = false;
	for (const auto& ext : Extensions)
	{
		if (InputPath.extension() == ext)
		{
			Supported = true;
			break;
		}
	}
	if (Supported == false)
	{
		return false;
	}

	//	出力パスの設定
	fs::path OutputPath = InputPath;
	OutputPath.replace_extension(L".aud");
	if (OverWrite == false && fs::exists(OutputPath))
	{
		std::wcout << L"Skiping: " << InputPath.filename() << std::endl;
	}

	//	デコーダ初期化
	ma_decoder_config Config = ma_decoder_config_init(ma_format_s16, 0, 0);
	ma_decoder Decoder;
	ma_result Result = ma_decoder_init_file_w(InputPath.c_str(), &Config, &Decoder);
	if (Result != MA_SUCCESS)
	{
		print_impl_ma_error(L"Failed to initialize decoder for" + InputPath.filename().wstring(), Result);
		return false;
	}

	//	出力ファイル展開
	std::ofstream ofs(OutputPath, std::ios::binary);
	if (!ofs)
	{
		std::wcerr << L"Failed to open output file: " << OutputPath << std::endl;
		ma_decoder_uninit(&Decoder);
		return false;
	}

	//	ヘッダ書き込み
	AudioHeader Header = {};
	Header.SampleRate = Decoder.outputSampleRate;
	Header.Channels = static_cast<uint16_t>(Decoder.outputChannels);
	Header.BitsPerSample = 16;
	Header.FrameCount = 0;
	ofs.write(reinterpret_cast<const char*>(&Header), sizeof(Header));

	//	チャンク処理
	//	16384フレーム = ステレオ16bitで約64KB
	constexpr ma_uint64 CHUNK_SIZE = 16384;
	std::vector<int16_t> Buffer(CHUNK_SIZE * Decoder.outputChannels);
	ma_uint64 TotalFramesRead = 0;

	while (true)
	{
		ma_uint64 FramesRead = 0;
		Result = ma_decoder_read_pcm_frames(&Decoder, Buffer.data(), CHUNK_SIZE, &FramesRead);

		if (FramesRead > 0)
		{
			ofs.write(reinterpret_cast<const char*>(Buffer.data()), FramesRead * Decoder.outputChannels * sizeof(int16_t));
			TotalFramesRead += FramesRead;
		}

		//	終端なら終了
		if (Result == MA_AT_END)
		{
			break;
		}

		//	成功判定
		if (Result != MA_SUCCESS)
		{
			print_impl_ma_error(L"Error during decoding" + InputPath.filename().wstring(), Result);
			break;
		}
	}

	//	ヘッダの確定
	Header.FrameCount = TotalFramesRead;
	ofs.seekp(0, std::ios::beg);
	ofs.write(reinterpret_cast<const char*>(&Header), sizeof(Header));

	ofs.close();
	ma_decoder_uninit(&Decoder);

	if (Result != MA_SUCCESS && Result != MA_AT_END)
	{
		fs::remove(OutputPath);
		return false;
	}

	std::wcout << L"Finish Convert To BinaryFile. : " << InputPath.filename() << std::endl;
	return true;
}
