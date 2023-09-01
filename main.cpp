#include "DxLib.h"

struct InitialScreen {
	int Width;			//スクリーンの横幅
	int Height;			//スクリーンの縦幅
	int colorBit;		//カラービット数
};

struct WritingScreen {
	int color;			//普通の描画を書き込むためのスクリーン
	int highBlight;		//高輝度を抜き出し書き込むためのスクリーン
	int reduction;		//highBlightScreenを縮小した物を書き込むためのスクリーン
	int gauss;			//reductionScreenにガウス処理を施した物を書き込むためのスクリーン
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	InitialScreen initScreen;
	initScreen.Width = 640;
	initScreen.Height = 480;
	initScreen.colorBit = 32;

	WritingScreen writingScreen;

	int reductionSize = 8;
	int reductionScreenWidth = initScreen.Width / reductionSize;
	int reductionScreenHeight = initScreen.Height / reductionSize;

	int gaussRation = 900;

	ChangeWindowMode(true);

	SetGraphMode(initScreen.Width, initScreen.Height, initScreen.colorBit);

	if (DxLib_Init() < 0) {
		return -1;
	}

	writingScreen.color = MakeScreen(initScreen.Width, initScreen.Height, false);
	writingScreen.highBlight = MakeScreen(initScreen.Width, initScreen.Height, false);
	writingScreen.reduction = MakeScreen(reductionScreenWidth, reductionScreenHeight, false);
	writingScreen.gauss = MakeScreen(reductionScreenWidth, reductionScreenHeight, false);

	SetUseLighting(false);

	SetBackgroundColor(0, 0, 0);

	SetCameraNearFar(5.0f, 500.0f);

	// Zバッファを使用する
	SetUseZBuffer3D(true);
	// Zバッファへの書き込みを行う
	SetWriteZBuffer3D(true);
	// ポリゴンの裏面を描画しない
	SetUseBackCulling(true);


	while (ProcessMessage() == 0) {

		
		// 上下キーが押されたらガウスフィルタのぼかし度合いを変更する
		if (CheckHitKey(KEY_INPUT_UP))
		{
			gaussRation++;
		}
		if (CheckHitKey(KEY_INPUT_DOWN))
		{
			if (gaussRation > 0)
				gaussRation--;
		}

		SetDrawScreen(writingScreen.color);
		ClearDrawScreen();

		SetCameraPositionAndTargetAndUpVec(VGet(0, 80, -200), VGet(0, 0, 0), VGet(0, 1, 0));

		DrawSphere3D(VGet(0, 0, 0), 32, 32, 0xffffff, 0xffffff, true);

		DrawCone3D(VGet(0, 64, 0), VGet(0, 0, 0), 48, 32, 0x333333, 0x333333, true);

		DrawLine3D(VGet(0, 0, -250), VGet(0, 0, 250), 0xff0000);

		GraphFilterBlt(writingScreen.color, writingScreen.highBlight, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_LESS, 230, true, 0x000000, 255);
		GraphFilterBlt(writingScreen.highBlight, writingScreen.reduction, DX_GRAPH_FILTER_DOWN_SCALE, reductionSize);
		GraphFilterBlt(writingScreen.reduction, writingScreen.gauss, DX_GRAPH_FILTER_GAUSS, 16, gaussRation);


		SetDrawScreen(DX_SCREEN_BACK);

		DrawGraph(0, 0, writingScreen.color, false);

		SetDrawMode(DX_DRAWMODE_BILINEAR);

		SetDrawBlendMode(DX_BLENDMODE_ADD, 255);

		DrawExtendGraph(0, 0, initScreen.Width, initScreen.Height, writingScreen.gauss, false);
		DrawExtendGraph(0, 0, initScreen.Width, initScreen.Height, writingScreen.gauss, false);

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

		SetDrawMode(DX_DRAWMODE_NEAREST);

		DrawFormatString(0, 0, 0xffffff, L"gauss:%d", gaussRation);

		ScreenFlip();
	}

	DxLib_End();

	return 0;
}