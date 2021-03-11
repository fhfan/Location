// MultiRegions.cpp : 实现文件
//

#include "stdafx.h"
#include "PHZNVisionApp.h"
#include "MultiRegions.h"
#include "afxdialogex.h"
#include<algorithm>

//extern "C" void __declspec(dllimport) HIOCancelDraw();

// MultiRegions 对话框

IMPLEMENT_DYNAMIC(MultiRegions, CDialogEx)

MultiRegions::MultiRegions(CWnd* pParent /*=NULL*/)
	: CDialogEx(MultiRegions::IDD, pParent)
	, m_isDrawing(false)
	, m_startX(0)
	, m_startY(0)
	, m_bImgMove(false)
	, m_isTypeChanged(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

MultiRegions::~MultiRegions()
{
}

void MultiRegions::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_XLDIN, m_list_xldIn);
	DDX_Control(pDX, IDC_COMBO_XLDADDED, m_combo_xldOut);
	DDX_Control(pDX, IDC_LIST_ALLREGION, m_list_detectAllRegion);
	DDX_Control(pDX, IDC_COMBO_PLOTTYPE, m_combo_roiplottype);
	DDX_Control(pDX, IDC_COMBO_DETECTMETHOD, m_combo_detectmethod);
	DDX_Control(pDX, IDC_COMBO_LIGHTDARK, m_combo_lightdark);
	DDX_Control(pDX, IDC_COMBO_REGIONINOUT, m_combo_regioninout);
}


BEGIN_MESSAGE_MAP(MultiRegions, CDialogEx)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_REGION_REFRESH,  &MultiRegions::OnBnClickedRegionRefresh)
	ON_BN_CLICKED(IDC_REGION_CONFIRM,  &MultiRegions::OnBnClickedRegionConfirm)
	ON_BN_CLICKED(IDC_BUTTON_PLOTXLDS, &MultiRegions::OnBnClickedButtonPlotxlds)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_XLDIN, &MultiRegions::OnLvnItemchangedListXldin)
	ON_BN_CLICKED(IDC_BUTTON_LOADIMAGE_XLD, &MultiRegions::OnBnClickedButtonLoadimageXld)
	ON_BN_CLICKED(IDC_BUTTON_IMAGECUT, &MultiRegions::OnBnClickedImagecut)
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_CBN_SELCHANGE(IDC_COMBO_REGIONINOUT, &MultiRegions::OnCbnSelchangeComboRegioninout)
	ON_CBN_SELCHANGE(IDC_COMBO_DETECTMETHOD, &MultiRegions::OnCbnSelchangeComboDetectmethod)
	ON_CBN_SELCHANGE(IDC_COMBO_LIGHTDARK, &MultiRegions::OnCbnSelchangeComboLightdark)
	ON_CBN_SELCHANGE(IDC_COMBO_XLDADDED, &MultiRegions::OnCbnSelchangeComboXldadded)
	ON_CBN_EDITUPDATE(IDC_COMBO_XLDADDED, &MultiRegions::OnCbnEditupdateComboXldadded)
	ON_CBN_SELCHANGE(IDC_COMBO_PLOTTYPE, &MultiRegions::OnCbnSelchangeComboPlottype)  //错误
	ON_BN_CLICKED(IDC_BUTTON1, &MultiRegions::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &MultiRegions::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &MultiRegions::OnBnClickedButton3)
END_MESSAGE_MAP()


// MultiRegions 消息处理程序


BOOL MultiRegions::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	CreateImageWindow();

	//待使用算法
	detectmethods.push_back(_T("对比算法"));
	detectmethods.push_back(_T("纯色算法"));
	detectmethods.push_back(_T("小孔算法"));
	((CComboBox*)GetDlgItem(IDC_COMBO_DETECTMETHOD))->ResetContent();
	for (int i = 0; i < detectmethods.size(); ++i)
		m_combo_detectmethod.AddString(detectmethods[i]);
	m_combo_detectmethod.SetCurSel(0);

	//待使用绘制ROI形状
	((CComboBox*)GetDlgItem(IDC_COMBO_PLOTTYPE))->ResetContent();
	m_combo_roiplottype.AddString(_T("任意形状"));
	m_combo_roiplottype.AddString(_T("矩形"));
	m_combo_roiplottype.AddString(_T("圆形"));
	m_combo_roiplottype.AddString(_T("椭圆"));
	m_combo_roiplottype.SetCurSel(0);

	//内黑外白、内白外黑
	((CComboBox*)GetDlgItem(IDC_COMBO_LIGHTDARK))->ResetContent();
	m_combo_lightdark.AddString(_T("内黑外白"));
	m_combo_lightdark.AddString(_T("内白外黑"));
	m_combo_lightdark.SetCurSel(0);

	//区域内，区域外
	((CComboBox*)GetDlgItem(IDC_COMBO_REGIONINOUT))->ResetContent();
	m_combo_regioninout.AddString(_T("区域外"));//0是外
	m_combo_regioninout.AddString(_T("区域内"));//1是内
	m_combo_regioninout.SetCurSel(0);

	//待抠掉区域
	CRect rectIn;
	m_list_xldIn.GetWindowRect(&rectIn);
	m_list_xldIn.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	//待检测区域
	CRect rectOut;
	m_list_detectAllRegion.GetWindowRect(&rectOut);
	m_list_detectAllRegion.SetExtendedStyle(m_list_detectAllRegion.GetExtendedStyle() | LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES);
	m_list_detectAllRegion.InsertColumn(0, _T("区域"), LVCFMT_CENTER, rectOut.Width() / 10);
	m_list_detectAllRegion.InsertColumn(1, _T("检测算法"), LVCFMT_CENTER, rectOut.Width() / 6);
	m_list_detectAllRegion.InsertColumn(2, _T("内外比对"), LVCFMT_CENTER, rectOut.Width() / 6);
	m_list_detectAllRegion.InsertColumn(3, _T("外轮廓"), LVCFMT_CENTER, rectOut.Width() / 8);
	m_list_detectAllRegion.InsertColumn(4, _T("内轮廓"), LVCFMT_LEFT, rectOut.Width() * 37 / 120);
	m_list_detectAllRegion.InsertColumn(5, _T("区域模式"), LVCFMT_LEFT, rectOut.Width() / 6);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

void MultiRegions::CreateImageWindow()
{
	HTuple HWindowID;
	//CRect Rect;
	CWnd* pWnd = GetDlgItem(IDC_PIC_MULTIREGION);
	HWindowID = (Hlong)pWnd->m_hWnd;
	pWnd->GetWindowRect(&Rect);
	OpenWindow(0, 0, Rect.Width(), Rect.Height(), HWindowID, "visible", "", &hvWindowID);
}


void MultiRegions::OnPaint()
{
	CRect rectDlg; CPaintDC dc(this);
	GetClientRect(rectDlg);
	dc.FillSolidRect(rectDlg, RGB(200, 200, 200));
	CDialogEx::OnPaint();
}

HBRUSH MultiRegions::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (pWnd->GetDlgCtrlID() == (IDC_STATIC_XLDADDED) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC_XLDIN) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC_LIGHTDARK) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC_DETECTMETHOD))
	{
		//MessageBox(_T("static text"));
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(41, 36, 33));
		return HBRUSH(GetStockObject(HOLLOW_BRUSH));
	}
	return hbr;
}


//void dxf_scale(void * hContours, double pixel_size, void *hContours_result, double *x_base, double *y_base)
//{
//	double scale = 1.0 / pixel_size;
//
//
//	HTuple  hv_HomMat2DIdentity;
//	HTuple  hv_Scale = scale;
//	HTuple  hv_HomMat2DScale;
//	HObject& ho_Contours = *(HObject*)hContours;
//	HObject& ho_ContoursScaled = *(HObject*)hContours_result;
//	HTuple  hv_PointOrder;
//
//	UnionAdjacentContoursXld(ho_Contours, &ho_Contours, 1, 1, "attr_keep");
//	HomMat2dIdentity(&hv_HomMat2DIdentity);
//	hv_Scale = scale;
//	HomMat2dScale(hv_HomMat2DIdentity, hv_Scale, hv_Scale, 0, 0, &hv_HomMat2DScale);
//	AffineTransContourXld(ho_Contours, &ho_ContoursScaled, hv_HomMat2DScale);
//
//	HTuple  hv_Area, hv_RowScaled, hv_ColScaled;
//
//	AreaCenterXld(ho_Contours, &hv_Area, &hv_RowScaled, &hv_ColScaled, &hv_PointOrder);
//	*x_base = hv_ColScaled[0];
//	*y_base = hv_RowScaled[0];
//}


void dxf_scale(void * hContours, double pixel_size, void *hContours_result, double *x_base, double *y_base)
{
	double scale = 1.0 / pixel_size;

	HTuple  hv_HomMat2DIdentity;
	HTuple  hv_Scale = scale;
	HTuple  hv_HomMat2DScale;
	HObject& ho_Contours = *(HObject*)hContours;
	HObject& ho_ContoursScaled = *(HObject*)hContours_result;
	HTuple  hv_PointOrder;

	UnionAdjacentContoursXld(ho_Contours, &ho_Contours, 1, 1, "attr_keep");
	HomMat2dIdentity(&hv_HomMat2DIdentity);
	hv_Scale = scale;
	HomMat2dScale(hv_HomMat2DIdentity, hv_Scale, hv_Scale, 0, 0, &hv_HomMat2DScale);
	AffineTransContourXld(ho_Contours, &ho_ContoursScaled, hv_HomMat2DScale);

	HTuple  hv_Area, hv_RowScaled, hv_ColScaled;

	AreaCenterXld(ho_Contours, &hv_Area, &hv_RowScaled, &hv_ColScaled, &hv_PointOrder);

	*x_base = hv_ColScaled.TupleMean().D();
	*y_base = hv_RowScaled.TupleMean().D();
}

//void dxf_transform(void* hContours, double xp, double yp, double angle, void* hContours_result)
//{
//	HTuple  hv_HomMat2DIdentity;
//	HTuple  hv_Area, hv_RowScaled, hv_ColScaled;
//	HObject& ho_Contours = *(HObject*)hContours;
//	HObject& ho_ContoursCenteredAndTranlated = *(HObject*)hContours_result;
//
//	HTuple  hv_PointOrder;
//	HTuple  hv_HomMat2D;
//
//	AreaCenterXld(ho_Contours, &hv_Area, &hv_RowScaled, &hv_ColScaled, &hv_PointOrder);
//	double x_center = hv_ColScaled[0];
//	double y_center = hv_RowScaled[0];
//
//	//double x_center = hv_ColScaled.TupleMean().D();
//	//double y_center = hv_RowScaled.TupleMean().D();
//
//	//printf("DXF mean center xy(%f,%f),at %s:%d\r\n", hv_ColScaled.TupleMean().D(), hv_RowScaled.TupleMean().D(), __FUNCTION__, __LINE__);
//	//printf("DXF center 0: xy(%f,%f),at %s:%d\r\n", x_center, y_center, __FUNCTION__, __LINE__);
//
//	VectorAngleToRigid(HTuple(hv_RowScaled[0]), HTuple(hv_ColScaled[0]), 0, yp,
//		xp, (angle * 3.1415926) / 180, &hv_HomMat2D);
//	AffineTransContourXld(ho_Contours, &ho_ContoursCenteredAndTranlated, hv_HomMat2D);
//
//	HTuple  hv_Area_, hv_RowScaled_, hv_ColScaled_;
//	HTuple  hv_PointOrder_;
//
//	AreaCenterXld(ho_ContoursCenteredAndTranlated, &hv_Area_, &hv_RowScaled_, &hv_ColScaled_, &hv_PointOrder_);
//	x_center = hv_ColScaled_[0];
//	y_center = hv_RowScaled_[0];
//
//	//printf("DXF mean center_ xy(%f,%f),at %s:%d\r\n", hv_ColScaled_.TupleMean().D(), hv_RowScaled_.TupleMean().D(), __FUNCTION__, __LINE__);
//	//printf("DXF center_ 0: xy(%f,%f),at %s:%d\r\n", x_center, y_center, __FUNCTION__, __LINE__);
//}


void dxf_transform(void* hContours, double xp, double yp, double angle, void* hContours_result)
{
	HTuple  hv_HomMat2DIdentity;
	HTuple  hv_Area, hv_RowScaled, hv_ColScaled;
	HObject& ho_Contours = *(HObject*)hContours;
	HObject& ho_ContoursCenteredAndTranlated = *(HObject*)hContours_result;

	HTuple  hv_PointOrder;
	HTuple  hv_HomMat2D;

	AreaCenterXld(ho_Contours, &hv_Area, &hv_RowScaled, &hv_ColScaled, &hv_PointOrder);
	double x_center = hv_ColScaled.TupleMean().D();
	double y_center = hv_RowScaled.TupleMean().D();

	//printf("DXF mean center xy(%f,%f),at %s:%d\r\n", x_center, y_center, __FUNCTION__, __LINE__);

	VectorAngleToRigid(HTuple(y_center), HTuple(x_center), 0, yp,
		xp, (angle * 3.1415926) / 180, &hv_HomMat2D);
	AffineTransContourXld(ho_Contours, &ho_ContoursCenteredAndTranlated, hv_HomMat2D);
#if 0
	HTuple  hv_Area_, hv_RowScaled_, hv_ColScaled_;
	HTuple  hv_PointOrder_;

	AreaCenterXld(ho_ContoursCenteredAndTranlated, &hv_Area_, &hv_RowScaled_, &hv_ColScaled_, &hv_PointOrder_);
	double x_center_ = hv_ColScaled_.TupleMean().D();
	double y_center_ = hv_RowScaled_.TupleMean().D();

	printf("DXF mean center_ xy(%f,%f),at %s:%d\r\n", x_center_, y_center_, __FUNCTION__, __LINE__);
#endif
}

//图像显示
void MultiRegions::DisPlayImage(HObject hImageSrc, HTuple hWindow)
{
	//判断hImageSrc是否加载图片初始化
	if (!hImageSrc.IsInitialized())
	{
		return;
	}

	//HTuple hImageWidth; //图片原始宽度
	//HTuple hImageHeight;//图片原始高度
	GetImageSize(hImageSrc, &m_hWidth, &m_hHeight);

	int nImageHeight = m_hHeight.I();
	int nImageWidth = m_hWidth.I();

	double Row1 = nImageHeight*m_dScale - m_nHeightOffset;
	double Column1 = nImageWidth*m_dScale - m_nWidthOffset;
	double Row2 = nImageHeight - nImageHeight*m_dScale - m_nHeightOffset;
	double Column2 = nImageWidth - nImageWidth*m_dScale - m_nWidthOffset;
	//GenRectangle1(&ho_Rectangle, Row1, Column1, Row2, Column2);
	//Difference(hImageSrc, ho_Rectangle, &ho_DiffRectangle);
	//ClearWindow(hWindow);
	SetColor(hWindow, "white");
	SetDraw(hWindow, "fill");
	//OverpaintRegion(hImageSrc, ho_DiffRectangle, 255, "fill");
	HDevWindowStack::Push(hWindow);
	if (HDevWindowStack::IsOpen())
	{
		DispRectangle1(hWindow, 0, 0, m_hHeight, m_hWidth);
		SetPart(hWindow, Row1, Column1, Row2, Column2);
		DispObj(hImageSrc, HDevWindowStack::GetActive());
	}

}

LPITEMIDLIST MultiRegions::ParsePidlFromPath(LPCSTR path)

{

	OLECHAR szOleChar[MAX_PATH];

	LPSHELLFOLDER IpsfDeskTop;

	LPITEMIDLIST lpifq;

	ULONG ulEaten, ulAttribs;

	HRESULT hres;

	SHGetDesktopFolder(&IpsfDeskTop);

	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, path, -1, szOleChar, sizeof(szOleChar));

	hres = IpsfDeskTop->ParseDisplayName(NULL, NULL, szOleChar, &ulEaten, &lpifq, &ulAttribs);

	hres = IpsfDeskTop->Release();

	if (FAILED(hres))

		return NULL;

	return lpifq;
}



void MultiRegions::OnBnClickedButtonLoadimageXld()
{
	//// TODO:  在此添加控件通知处理程序代码
	//if (m_isDrawing)
	//{
	//	AfxMessageBox("请先停止绘图！\n请在绘图区点击鼠标右键后再加载！");
	//	return;
	//}

	//b_reduceList.clear();
	//hv_singleRegionList.clear();
	//hv_single_XldList.clear();

	//HTuple m_ImageWidth, m_ImageHeight, hv_ImagePath = "Rsult01.bmp";
	//ReadImage(&hoImage, hv_ImagePath);
	//GetImageSize(hoImage, &m_ImageWidth, &m_ImageHeight);
	//int img_w = m_ImageWidth; int img_h = m_ImageHeight;
	//int picctrl_w = PicRect.Width(); int picctrl_h = PicRect.Height();
	//float fImage = img_w / img_h;
	//float fWindow = picctrl_w / picctrl_h;
	//float w = fWindow * img_h;
	//float h = img_w / fWindow;

	//if (fWindow > fImage)
	//{
	//	m_dDispImagePartRow0 = 0;
	//	m_dDispImagePartCol0 = -(w - img_w) / 2;
	//	m_dDispImagePartRow1 = img_h - 1;
	//	m_dDispImagePartCol1 = img_w + (w - img_w) / 2;
	//}
	//else
	//{
	//	m_dDispImagePartRow0 = -(h - img_h) / 2;
	//	m_dDispImagePartCol0 = 0;
	//	m_dDispImagePartRow1 = img_h + (h - img_h) / 2;
	//	m_dDispImagePartCol1 = img_w - 1;
	//}
	//ShowImage();

	////if (fWindow > fImage)
	////	SetPart(hvWindowID, 0, -(w - img_w) / 2, img_h-1, img_w + (w - img_w) / 2);
	////else
	////	SetPart(hvWindowID, -(h - img_h) / 2, 0, img_h + (h - img_h) / 2, img_w-1);

	//HDevWindowStack::Push(hvWindowID);
	//if (HDevWindowStack::IsOpen()) DispObj(hoImage, HDevWindowStack::GetActive());

	////清除内部轮廓与待检测区域列表内容
	//m_combo_xldOut.ResetContent();
	//m_list_xldIn.DeleteAllItems();
	//m_list_detectAllRegion.DeleteAllItems();

	// TODO:  在此添加控件通知处理程序代码
	HTuple hv_Files, hv_DxfStatus, hv_ReadPath;
	CString str, target;

	//BROWSEINFO bi;
	//char name[MAX_PATH];
	//ZeroMemory(&bi, sizeof(BROWSEINFO));
	//LPITEMIDLIST rootLoation = ParsePidlFromPath(".\\location");
	//bi.hwndOwner = GetSafeHwnd();
	//bi.pszDisplayName = name;
	////bi.lpszTitle = "S浏览文件夹";
	//bi.ulFlags = BIF_RETURNONLYFSDIRS;

	//ZeroMemory(&bi, sizeof(bi));
	//bi.pidlRoot = rootLoation;
	//bi.lpszTitle = _T(""); // 可以不指定
	//LPITEMIDLIST targetLocation = SHBrowseForFolder(&bi);
	//if (targetLocation != NULL) {
	//	SHGetPathFromIDList(targetLocation, (LPSTR)(LPCSTR)target);
	//}
	//else
	//	return;

	TCHAR szBuffer[MAX_PATH] = { 0 };
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = NULL;
	bi.pszDisplayName = szBuffer;
	bi.lpszTitle = _T("从下面选择文件或文件夹:");
	bi.ulFlags = BIF_BROWSEINCLUDEFILES;
	LPITEMIDLIST idl = SHBrowseForFolder(&bi);
	if (NULL == idl)
	{
		return;
	}
	SHGetPathFromIDList(idl, szBuffer);
	//hv_ReadPath = HTuple(target);
	//ListFiles(hv_ReadPath, "files", &hv_Files);
	ReadImage(&hoImage, HTuple(szBuffer));
	//GetImagePointer1(hoImage, NULL, NULL, &m_ImageWidth, &m_ImageHeight);
	GetImageSize(hoImage, &m_hWidth, &m_hHeight);
	SetPart(hvWindowID, 0, 0, m_hHeight - 1, m_hWidth - 1);
	//设置窗口

	/*float fImage = m_hWidth.D() / m_hHeight.D();

	float fWindow = (float)m_rtImage.Width() / m_rtImage.Height();

	float Row0 = 0, Col0 = 0, Row1 = m_hHeight - 1, Col1 = m_hWidth - 1;

	if (fWindow > fImage)

	{
	float w = fWindow * m_hHeight;

	Row0 = 0;

	Col0 = -(w - m_hWidth) / 2;

	Row1 = m_hHeight - 1;

	Col1 = m_hWidth + (w - m_hWidth) / 2;

	}

	else

	{
	float h = m_hWidth / fWindow;

	Row0 = -(h - m_hHeight) / 2;

	Col0 = 0;

	Row1 = m_hHeight + (h - m_hHeight) / 2;

	Col1 = m_hWidth - 1;

	}

	m_dDispImagePartRow0 = Row0;

	m_dDispImagePartCol0 = Col0;

	m_dDispImagePartRow1 = Row1;

	m_dDispImagePartCol1 = Col1;

	ShowImage();*/
	/*ReadContourXldDxf(&ho_Contours, ".\\location\\工程图文档5.dxf", HTuple(),
		HTuple(), &hv_DxfStatus);*/
	//dxf_scale(&ho_Contours, pixel_size, &ho_ContoursScaled, &x_base, &y_base);

	HDevWindowStack::Push(hvWindowID);
	if (HDevWindowStack::IsOpen())
	{
		DispObj(hoImage, HDevWindowStack::GetActive());
		//DispObj(ho_ContoursScaled, HDevWindowStack::GetActive());
		//while (true)
		//{
		//	/*GetMposition(hvWindowID, &hv_mouseR, &hv_mouseC, &hv_Button);
		//	cout << "R:" << hv_mouseR.I() << endl;
		//	cout << "C:" << hv_mouseC.I() << endl;*/
		//	dxf_transform(&ho_ContoursScaled, hv_mouseC.I(), hv_mouseR.I(), angle, &ho_ModelTrans);
		//	DispObj(ho_ModelTrans, HDevWindowStack::GetActive());
		//}
	}
}

void MultiRegions::OnBnClickedButtonPlotxlds()
{
	// TODO:  在此添加控件通知处理程序代码
	//获取需要绘制的ROI形状
	//if (!m_isDrawing)
	//{
	//	GetDlgItem(IDC_BUTTON_PLOTXLDS)->SetWindowText("停止绘制");
	//}
	//else
	//{
	//	GetDlgItem(IDC_BUTTON_PLOTXLDS)->SetWindowText("开始绘制");
	//	//HIOCancelDraw();
	//}

	//m_isDrawing = !m_isDrawing;

	Drawing();

	return;
}

void MultiRegions::Drawing()
{

	/*if (!m_isDrawing)
	{
		return;
	}*/

	/*while (m_isDrawing)
	{*/
		//CString roitype = "";
		//int index = ((CComboBox*)GetDlgItem(IDC_COMBO_PLOTTYPE))->GetCurSel();
		//((CComboBox*)GetDlgItem(IDC_COMBO_PLOTTYPE))->GetLBText(index, roitype);

	HObject ho_Region1, Contours1;
	HObject ho_Region, Contours;
	HTuple hv_RowXld, hv_ColXld, hv_String, hv_Number, hv_I;
		//if (HDevWindowStack::IsOpen()) SetColor(HDevWindowStack::GetActive(), "red");
		//HTuple hv_Row, hv_Column, hv_Phi, hv_Length1, hv_Length2, hv_Radius, hv_Radius1, hv_Radius2;

		//if (roitype == _T("任意形状"))
		//{
		//	DrawRegion(&ho_Region, hvWindowID);
		//}
		//else if (roitype == _T("矩形"))
		//{
		//	DrawRectangle2(hvWindowID, &hv_Row, &hv_Column, &hv_Phi, &hv_Length1, &hv_Length2);
		//	GenRectangle2(&ho_Region, hv_Row, hv_Column, hv_Phi, hv_Length1, hv_Length2);
		//}
		//else if (roitype == _T("圆形"))
		//{
		//	DrawCircle(hvWindowID, &hv_Row, &hv_Column, &hv_Radius);
		//	GenCircle(&ho_Region, hv_Row, hv_Column, hv_Radius);
		//}
		//else if (roitype == _T("椭圆"))
		//{
		//	DrawEllipse(hvWindowID, &hv_Row, &hv_Column, &hv_Phi, &hv_Radius1, &hv_Radius2);
		//	GenEllipse(&ho_Region, hv_Row, hv_Column, hv_Phi, hv_Radius1, hv_Radius2);
		//}

		//if (!m_isDrawing)
		//{
		//	break;
		//}

		////绘图类型已经变更，放弃本次操作
		//if (m_isTypeChanged)
		//{
		//	m_isTypeChanged = false;
		//	continue;
		//}
	SelectObj(ho_GenContours, &Contours1, hv_contourGroupIdx);
	SelectObj(ho_GenRegions, &ho_Region1, hv_contourGroupIdx);
	//GenContourRegionXld(ho_Region, &Contours, "border");
	//GetContourXld(Contours, &hv_RowXld, &hv_ColXld);

	/*if (!ho_Region.IsInitialized() || !Contours.IsInitialized())
	{
		continue;
	}*/

	/*bool checked = 0;
	b_reduceList.push_back(checked);
	for (int i = 0; i < b_reduceList.size(); ++i) b_reduceList[i] = checked;*/
	//fill(b_reduceList.begin(), b_reduceList.end(),"false");

	//已添加轮廓下拉列表显示刷新
	//int nCurRangingSize = 0;
	CountObj(Contours1, &hv_Number);
	HalconCpp::HTuple end_val30 = hv_Number;
	HalconCpp::HTuple step_val30 = 1;
	//b_reduceList.clear();
	//hv_single_XldList.clear();
	//hv_singleRegionList.clear();
	for (hv_I = 1; hv_I.Continue(end_val30, step_val30); hv_I += step_val30)
	{
		SelectObj(Contours1, &Contours, hv_I);
		SelectObj(ho_Region1, &ho_Region, hv_I);
		GetContourXld(Contours, &hv_RowXld, &hv_ColXld);
		bool checked = 0;
		b_reduceList.push_back(checked);
		for (int i = 0; i < b_reduceList.size(); ++i) b_reduceList[i] = checked;
		int nCurRangingSize = 0;
		hv_single_XldList.push_back(Contours);
		hv_singleRegionList.push_back(ho_Region);
		nCurRangingSize = hv_single_XldList.size();
		string strTemp, strRangingName;
		strRangingName = "XLD";
		m_combo_xldOut.ResetContent();//消除现有所有内容
		for (int i = 1; i <= nCurRangingSize; i++)
		{
			strTemp = strRangingName + to_string(i);
			m_combo_xldOut.InsertString(-1, strTemp.c_str());
		}
		m_combo_xldOut.SetCurSel(nCurRangingSize - 1);
		ReListData(nCurRangingSize);

		//轮廓及对应名称显示于控件
		try
		{
			float x_xld = hv_RowXld; float y_xld = hv_ColXld;
			CString xldname;
			xldname.Format(_T("区域%d"), nCurRangingSize);
			if (HDevWindowStack::IsOpen()) SetColor(HDevWindowStack::GetActive(), "spring green");
			SetTposition(hvWindowID, x_xld, y_xld);
			WriteString(hvWindowID, HTuple(xldname));
		}
		catch (...)
		{

		}

		if (HDevWindowStack::IsOpen()) SetColor(HDevWindowStack::GetActive(), "red");
		if (HDevWindowStack::IsOpen()) DispObj(Contours, HDevWindowStack::GetActive());
	}
		//Sleep(1000);

	//}

}

void MultiRegions::OnLvnItemchangedListXldin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	if (pNMLV->uChanged == LVIF_STATE)
	{
		int nTempNo = pNMLV->iItem;
		if ((pNMLV->uOldState & INDEXTOSTATEIMAGEMASK(1)) && (pNMLV->uNewState & INDEXTOSTATEIMAGEMASK(2)))
		{
			//排除自身
			if (nTempNo != m_combo_xldOut.GetCurSel())
			{
				b_reduceList[nTempNo] = 1;
			}
			//cout << "勾选第" << nTempNo + 1 << "个轮廓" << "\n";
			//cout << "待抠取轮廓情况为" << "" << "\n";
			//for (int i = 0; i < b_reduceList.size(); i++) cout << b_reduceList[i] << " ";
			//cout << "\n";

			//更新子区域check状态
			m_list_detectAllRegion.SetCheck(nTempNo, 0);
		}
		else if ((pNMLV->uOldState & INDEXTOSTATEIMAGEMASK(2)) && (pNMLV->uNewState & INDEXTOSTATEIMAGEMASK(1)))
		{
			b_reduceList[nTempNo] = 0;
			//cout << "取消勾选第" << nTempNo + 1 << "个轮廓" << "\n";
			//cout << "待抠取轮廓情况为" << "" << "\n";
			//for (int i = 0; i < b_reduceList.size(); i++) cout << b_reduceList[i] << " ";
			//cout << "\n";

			//更新子区域check状态
			m_list_detectAllRegion.SetCheck(nTempNo, 1);
		}
	}

	OnBnClickedRegionRefresh();

	*pResult = 0;
}

void MultiRegions::ReListData(int NumXld)
{
	//实时刷新listcontrol控件显示内容
	//m_list_xldIn.DeleteAllItems();
	//m_list_detectAllRegion.DeleteAllItems();
	LockWindowUpdate();

	//内部轮廓列表刷新
	DWORD dwStyle = m_list_xldIn.GetExStyle();
	m_list_xldIn.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	m_list_xldIn.InsertColumn(0, _T("NO"), LVCFMT_CENTER, 100);
	m_list_xldIn.DeleteAllItems();
	for (int i = 0; i < NumXld; i++)
	{
		CString s1;
		s1.Format(_T("XLD%d"), i + 1);
		m_list_xldIn.InsertItem(i, s1);
	}

	//待检测区域列表刷新
	CString s2;
	s2.Format(_T("%d"), NumXld);
	m_list_detectAllRegion.InsertItem(NumXld, s2);
	m_list_detectAllRegion.SetItemText(NumXld - 1, 1, detectmethods[0]);
	m_list_detectAllRegion.SetItemText(NumXld - 1, 2, _T("内黑外白"));
	m_list_detectAllRegion.SetItemText(NumXld - 1, 3, s2);
	m_list_detectAllRegion.SetItemText(NumXld - 1, 5, _T("区域内"));
	m_list_detectAllRegion.SetCheck(NumXld - 1, 1);//默认为勾选

	UnlockWindowUpdate();
}

void MultiRegions::OnBnClickedRegionRefresh()
{
	// TODO:  在此添加控件通知处理程序代码

	//1.获取轮廓序列
	CString strInfo1;
	int n_xldout = m_combo_xldOut.GetCurSel();
	cout << "选中待抠掉的外轮廓为 " << n_xldout + 1 << "\n";
	m_combo_xldOut.GetLBText(n_xldout, strInfo1);

	//2.获取检测算法
	CString strInfo2;
	int n_detectmethod = m_combo_detectmethod.GetCurSel();
	cout << "选中算法类型为 " << n_detectmethod + 1 << "\n";
	m_combo_detectmethod.GetLBText(n_detectmethod, strInfo2);

	//3.获取待抠掉轮廓
	vector<CString> xldchecked;
	for (int i = 0; i < b_reduceList.size(); ++i)
	{
		if (b_reduceList[i])
		{
			CString num_xld;
			num_xld.Format(_T("%d"), i + 1);
			xldchecked.push_back(num_xld);
		}
	}
	cout << "选中待抠掉的轮廓为 " ;
	int i; CString _xlds;
	for (i = 0; i < xldchecked.size(); i++)
	{
		_xlds += xldchecked[i];
		_xlds += _T(" ");
		cout << xldchecked[i] << " ";
	}
	cout << "\n" << "\n";

	//4.获取轮廓内外黑白状态
	CString strInfo3;
	int n_lightdark = m_combo_lightdark.GetCurSel();
	m_combo_lightdark.GetLBText(n_lightdark, strInfo3);
	cout << "轮廓内外黑白状态为   " << strInfo3 << "\n";

	CString strInfo4;
	int n_regioninout= m_combo_regioninout.GetCurSel();
	m_combo_regioninout.GetLBText(n_regioninout, strInfo4);
	cout << "区域模式为   " << strInfo4 << "\n";

	m_list_detectAllRegion.SetItemText(n_xldout, 1, detectmethods[n_detectmethod]);
	m_list_detectAllRegion.SetItemText(n_xldout, 2, strInfo3);
	m_list_detectAllRegion.SetItemText(n_xldout, 4, _xlds);
	m_list_detectAllRegion.SetItemText(n_xldout, 5, strInfo4);
	//更新check状态
	for (int i = 0; i < b_reduceList.size(); ++i)
	{
		if (IsChild(i + 1))
		{
			m_list_detectAllRegion.SetCheck(i, 0);
		}
		else
		{
			m_list_detectAllRegion.SetCheck(i, 1);
		}
	}
}

//判断是否子区域
bool MultiRegions::IsChild(int index)
{
	bool isChild = false;
	for (int i = 0; i < m_list_detectAllRegion.GetItemCount(); i++)
	{
		CString numxldin = m_list_detectAllRegion.GetItemText(i, 4);
		CStringArray regionIn; regionIn.SetSize(50);
		int n = SplitCString(numxldin, ' ', regionIn);

		for (int j = 0; j < n - 1; j++)
		{
			if (index == atoi(regionIn.GetAt(j)))
			{
				isChild = true;
				break;
			}
		}
	}

	return isChild;
}

//CString的split函数封装
int SplitCString(const CString str, char split, CStringArray &strArray)
{
	strArray.RemoveAll();
	CString strTemp = str; int iIndex = 0;
	while (1)
	{
		iIndex = strTemp.Find(split);
		if (iIndex >= 0)
		{
			strArray.Add(strTemp.Left(iIndex));
			strTemp = strTemp.Right(strTemp.GetLength() - iIndex - 1);
		}
		else break;
	}
	strArray.Add(strTemp);
	return strArray.GetSize();
}

void MultiRegions::OnBnClickedRegionConfirm()
{
	// TODO:  在此添加控件通知处理程序代码
	usedmodel.AllRegion.AllRegionImage.clear();
	usedmodel.singleRegionList.clear();
	usedmodel.AllDetectMethod.clear();
	usedmodel.AllLightDark.clear();
	usedmodel.theContourMainXLD.clear();
	usedmodel.NumsAllRegion.clear();
	usedmodel.AllRegionModel.clear();
	usedmodel.MaskRegionB.clear(); //环形区域
	usedmodel.MaskRegionA.clear();//非环区域
	//所有检测轮廓、区域推入容器
	for (int i = 0; i < hv_single_XldList.size(); ++i)
	{
		RegionBox singlexld;
		singlexld.mainXld = hv_single_XldList[i];
		HTuple Row1, Col1, Row2, Col2;
		SmallestRectangle1(hv_singleRegionList[i], &Row1, &Col1, &Row2, &Col2);
		singlexld.RowLeft = Row1;
		singlexld.RowRight = Col1;
		singlexld.ColLeft = Row2;
		singlexld.ColRight = Col2;
		singlexld.CntpntRow = (Row2 + Row1) / 2;
		singlexld.CntpntCol = (Col2 + Col1) / 2;
		singlexld.Weight = Col2 - Col1;
		singlexld.Height = Row2 - Row1;
		//usedmodel.theContourMainXLD.push_back(singlexld);
	}
	//usedmodel.theContourMainXLD.insert(usedmodel.theContourMainXLD.end(), hv_single_XldList.begin(), hv_single_XldList.end());
	usedmodel.singleRegionList.insert(usedmodel.singleRegionList.end(), hv_singleRegionList.begin(), hv_singleRegionList.end());
	
	int num_regions = m_list_detectAllRegion.GetItemCount();
	cout << "区域数量为： " << num_regions << "\n";
	for (int i = 0; i < num_regions; ++i)
	{
		//子区域不保存
		if (!m_list_detectAllRegion.GetCheck(i))
		{
			continue;
		}

		//所有执行的算法推入容器
		int nPosition = 0;
		CString numdetectmethod = m_list_detectAllRegion.GetItemText(i, 1);
		vector <CString>::iterator iElement = find(detectmethods.begin(),
			detectmethods.end(), numdetectmethod);
		if (iElement != detectmethods.end())
			nPosition = distance(detectmethods.begin(), iElement);
		usedmodel.AllDetectMethod.push_back(nPosition);	

		//所有区域内外黑白状态推入容器
		CString lightdark = m_list_detectAllRegion.GetItemText(i, 2);
		if (lightdark == _T("内黑外白")) 
			usedmodel.AllLightDark.push_back(0);
		else if (lightdark == _T("内白外黑"))
			usedmodel.AllLightDark.push_back(1);
		//所有区域内外压入容器
		CString regionmodel = m_list_detectAllRegion.GetItemText(i, 5);
		if (regionmodel == _T("区域外"))
			usedmodel.AllRegionModel.push_back(0);
		else if (regionmodel == _T("区域内"))
			usedmodel.AllRegionModel.push_back(1);
		//所有检测区域序列推入容器
		vector<int> serialNumberRegion;
		NumNode    NumberRegion;
		NumberRegion.outNum = 0;
		NumberRegion.inNum.clear();
		serialNumberRegion.clear();
		CString numxldout = m_list_detectAllRegion.GetItemText(i, 3);
		CString temp1;
		temp1 += numxldout;
		serialNumberRegion.push_back(atoi(temp1));//外轮廓编号
		NumberRegion.outNum = atoi(temp1);
		CString numxldin = m_list_detectAllRegion.GetItemText(i, 4);//内轮廓
		CStringArray regionIn; regionIn.SetSize(50);
		int n = SplitCString(numxldin, ' ', regionIn);

		for (int j = 0; j < n - 1; j++)
		{
			//cout << "atoi(regionIn[i])  " << regionIn.GetAt(j) << "\n";	
			serialNumberRegion.push_back(atoi(regionIn.GetAt(j)));
			NumberRegion.inNum.push_back(atoi(regionIn.GetAt(j)));
		}
		usedmodel.NumsAllRegion.push_back(NumberRegion);//内外轮廓编号
	}
	/*cout << "最后确定检测的全部区域" << "\n";
	for (int i = 0; i < usedmodel.NumsAllRegion.size(); ++i)
	{
	cout << "区域" << i + 1 << ": ";
	for (int j = 0; j < usedmodel.NumsAllRegion[i].size(); ++j)
	{
	cout << usedmodel.NumsAllRegion[i][j] << " ";
	}
	cout << "\n";
	}
	*/

	//所有检测区域推入容器	

	for (int i = 0; i < usedmodel.NumsAllRegion.size(); ++i)
	{
		HObject singleRegion, oneRegion, twoRegion, threeRegion, ho_ImageReduced, OutRegion;
		//HObject outRegion, inRegion, maskRegion, ho_outImage;
		HTuple hv_Rows, hv_Columns;
		HTuple  hv_Index, hv_I, hv_J, hv_Value;
		vector<HalconCpp::HObject> doubleRegion;
		HObject outRegion, inRegion;
		HObject Region = usedmodel.singleRegionList[usedmodel.NumsAllRegion[i].outNum-1];

		DilationCircle(Region, &outRegion, 10);
		usedmodel.AllRegion.AllRegionImage.push_back(outRegion);//所有区域
		RegionBox singlexld;
		HTuple Row1, Col1, Row2, Col2;
		SmallestRectangle1(outRegion, &Row1, &Col1, &Row2, &Col2);
		singlexld.RowLeft = Row1;
		singlexld.RowRight = Col1;
		singlexld.ColLeft = Row2;
		singlexld.ColRight = Col2;
		singlexld.CntpntRow = (Row2 + Row1) / 2;
		singlexld.CntpntCol = (Col2 + Col1) / 2;
		singlexld.Weight = Col2 - Col1;
		singlexld.Height = Row2 - Row1;
		usedmodel.theContourMainXLD.push_back(singlexld);
		ErosionCircle(Region, &inRegion, 10);
		Difference(outRegion, inRegion, &OutRegion);
		doubleRegion.push_back(OutRegion);
		for (int j = 0; j < usedmodel.NumsAllRegion[i].inNum.size(); ++j)
		{
			HObject differenceRegion;
			CreateDifferenceRegion(usedmodel.singleRegionList[usedmodel.NumsAllRegion[i].inNum[j]-1], differenceRegion);
			doubleRegion.push_back(differenceRegion);
		}
		singleRegion = usedmodel.AllRegion.AllRegionImage[i];
		for (int j = 0; j <doubleRegion.size(); ++j)
		{
			Difference(singleRegion, doubleRegion[j], &singleRegion);

		}
		usedmodel.MaskRegionB.push_back(doubleRegion); //环形区域
		usedmodel.MaskRegionA.push_back(singleRegion);//非环区域
	}

	cout << "轮廓总数量为： " << usedmodel.theContourMainXLD.size() << "\n";
	cout << "检测算法总数量为： " << usedmodel.AllDetectMethod.size() << "\n";
	cout << "检测轮廓内外状态总数量为： " << usedmodel.AllLightDark.size() << "\n";
	cout << "检测区域总数量为： " << usedmodel.AllRegion.AllRegionImage.size() << "\n";
	cout << "检测区域序列组总数量为： " << usedmodel.NumsAllRegion.size() << "\n";
}
void MultiRegions::OnBnClickedImagecut()
{
	// TODO:  在此添加控件通知处理程序代码
	//if (HDevWindowStack::IsOpen())
	//	HalconCpp::ClearWindow(HDevWindowStack::Pop());
	//HDevWindowStack::Push(hvWindowID);
	///*for (int i = 0; i < usedmodel.AllRegion.size(); ++i)
	//{
	//ReduceDomain(hoImage, usedmodel.AllRegion[i], &hoImageResult);
	//}*/
	//ReduceDomain(hoImage, usedmodel.AllRegion[0], &hoImageResult);
	//if (HDevWindowStack::IsOpen())
	//	DispObj(hoImageResult, HDevWindowStack::GetActive());

	//usedmodel.AllImageResult.clear();
	//for (int i = 0; i < usedmodel.AllRegion.AllRegionImage.size(); ++i)
	//{
	//	ReduceDomain(hoImage, usedmodel.AllRegion.AllRegionImage[i], &hoImageResult);
	//	usedmodel.AllImageResult.push_back(hoImageResult);
	//}

	//生成匹配模型
	HObject  ho_ImageReduced, hoImage_copy;
	HObject  ho_Region, ho_RegionDilation, ho_RegionDifference;
	HObject  ho_rectRegion, ho_MargeRegion, ho_outImage;

	HTuple  hv_UsedThreshold, hv_Mean, hv_Deviation, hv_Row1;
	HTuple  hv_Column1, hv_Row2, hv_Column2, hv_Rows, hv_Columns;
	HTuple  hv_Index, hv_I, hv_J;


	usedmodel.AllRegion.modelMatch.clear();
	usedmodel.AllRegion.modelContrast.clear();
	//usedmodel.AllRegion.modelMask1.clear();
	usedmodel.AllRegion.modelMask.clear();
	for (int i = 0; i < usedmodel.AllRegion.AllRegionImage.size(); ++i)
	{
		CopyImage(hoImage, &hoImage_copy);
		HTuple hv_Index = i;
		//生成匹配模型
		CreateModelMatch(hoImage_copy, usedmodel.AllRegion.AllRegionImage[i], hoImageResult, usedmodel.AllLightDark[i], usedmodel.AllRegionModel[i]);
		usedmodel.AllRegion.modelMatch.push_back(hoImageResult);
		CString name_match, name_contrast;
		//name_match.Format("modelmatch%d.bmp", i);
		name_contrast.Format("modelcontrast%d.bmp", i);
		HTuple hv_namematch = name_contrast;
		WriteImage(usedmodel.AllRegion.modelMatch[i], "bmp", 0, hv_namematch);
		//生成对比模型
		//CreateModelContrast(hoImageResult, hoImage, usedmodel.AllLightDark[i]);
		//usedmodel.AllRegion.modelContrast.push_back(hoImage);
		//HTuple hv_namecontrast = name_contrast;
		//WriteImage(usedmodel.AllRegion.modelContrast[0], "bmp", 0, hv_namecontrast);
		//	HTuple path2 = "ResultImage/Cpmpare" + hv_Index + ".bmp";
		//WriteImage(hoImage, "bmp", 0, path2);
		cv::Mat matTpm = visionFunLib.HObject2Mat(hoImage);
		cv::Mat matHpm = visionFunLib.HObject2Mat(hoImageResult);
		//cv::Mat matMask1(matTpm.rows, matTpm.cols, CV_8UC1);
		cv::Mat matMask2(matTpm.rows, matTpm.cols, CV_8UC1);
		cv::Mat matMask1 = cv::Mat::zeros(matTpm.rows, matTpm.cols, CV_8UC1);
		/*cv::Mat matMaskB(matTpm.rows,matTpm.cols,CV_8UC1);*/
		HObject mask = visionFunLib.Mat2HObject(matMask1);
		HObject modelMask;	
		//WriteImage(mask, "bmp", 0, "./mask.bmp");

		// HObject mask = visionFunLib.Mat2HObject(matMask2);
		//	CreateModeMask(hoImageResult, maskA, maskB, usedmodel.AllLightDark[i],usedmodel.AllRegionModel[i]);
		CreateModeMask(usedmodel.AllRegion.AllRegionImage[i], usedmodel.MaskRegionA[i], usedmodel.MaskRegionB[i], mask, modelMask, usedmodel.AllLightDark[i], usedmodel.AllRegionModel[i]);
		//usedmodel.AllRegion.modelMask1.push_back(maskA);
		usedmodel.AllRegion.modelMask.push_back(modelMask);
		//WriteImage(modelMask, "bmp", 0, "./modelMask.bmp");
		CString name_mask, name_maskB;
		name_mask.Format("modemask%d.bmp", i);
		//	name_maskB.Format("modemask%dB.bmp", i);
		HTuple hv_namemask = name_mask;
		//HTuple hv_namemaskB = name_maskB;
		WriteImage(usedmodel.AllRegion.modelMask[i], "bmp", 0, hv_namemask);
		//WriteImage(usedmodel.AllRegion.modelMask2[i], "bmp", 0, hv_namemaskB);
	}

	cout << "匹配模型总数量为： " << usedmodel.AllRegion.modelMatch.size() << "\n";
	cout << "对比模型总数量为： " << usedmodel.AllRegion.modelContrast.size() << "\n";
	//if (HDevWindowStack::IsOpen())
	//	HalconCpp::ClearWindow(HDevWindowStack::Pop());
	//HDevWindowStack::Push(hvWindowID);
	//if (HDevWindowStack::IsOpen())
	//	DispObj(usedmodel.AllRegion.modelMatch[0], HDevWindowStack::GetActive());
	//Sleep(4000);
	//if (HDevWindowStack::IsOpen())
	//	HalconCpp::ClearWindow(HDevWindowStack::Pop());
	//HDevWindowStack::Push(hvWindowID);
	//if (HDevWindowStack::IsOpen())
	//	DispObj(usedmodel.AllRegion.modelContrast[0], HDevWindowStack::GetActive());
}

void MultiRegions::ShowImage()
{
	if (hvWindowID != 0)
	{
		SetSystem("flush_graphic", "false");
		ClearWindow(hvWindowID);
		//显示
		if (hoImage.IsInitialized())
		{
			SetPart(hvWindowID, m_dDispImagePartRow0, m_dDispImagePartCol0, m_dDispImagePartRow1 - 1, m_dDispImagePartCol1 - 1);
			DispObj(hoImage, hvWindowID);
			if (hv_single_XldList.size() != 0)
			{
				for (int i = 0; i < hv_single_XldList.size(); ++i)
				{	
					if (HDevWindowStack::IsOpen())
						SetColor(HDevWindowStack::GetActive(), "red");			
					DispObj(hv_single_XldList[i], HDevWindowStack::GetActive());
					HTuple hv_RowXld, hv_ColXld;
					GetContourXld(hv_single_XldList[i], &hv_RowXld, &hv_ColXld);
					HTuple rowLength; 
					HTuple colLength;
					TupleLength(hv_RowXld, &rowLength);
					TupleLength(hv_ColXld, &colLength);
					if (rowLength.I() != 0 && colLength.I() != 0)
					{
						float x_xld = hv_RowXld; float y_xld = hv_ColXld;
						CString xldname;
						xldname.Format(_T("区域%d"), i + 1);
						if (HDevWindowStack::IsOpen())
							SetColor(HDevWindowStack::GetActive(), "spring green");
						SetTposition(hvWindowID, x_xld, y_xld);
						WriteString(hvWindowID, HTuple(xldname));
					}
				}				
			}			
		}
		SetSystem("flush_graphic", "true");
		HObject emptyObject;
		emptyObject.GenEmptyObj();
		DispObj(emptyObject, hvWindowID);
	}
}

BOOL MultiRegions::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	//CRect rtImage;
	//GetDlgItem(IDC_PIC_MULTIREGION)->GetWindowRect(&rtImage);
	/*if (PicRect.PtInRect(pt) && hoImage.IsInitialized())
	{
		Hlong  ImagePtX, ImagePtY;
		Hlong  Row0, Col0, Row1, Col1;
		double Scale = 0.1;
		
		if (zDelta<0)
		{
			ImagePtX = m_dDispImagePartCol0 + (pt.x - PicRect.left) / (PicRect.Width() - 1.0)*(m_dDispImagePartCol1 - m_dDispImagePartCol0);
			ImagePtY = m_dDispImagePartRow0 + (pt.y - PicRect.top) / (PicRect.Height() - 1.0)*(m_dDispImagePartRow1 - m_dDispImagePartRow0);
			Row0 = ImagePtY - 1 / (1 - Scale)*(ImagePtY - m_dDispImagePartRow0);
			Row1 = ImagePtY - 1 / (1 - Scale)*(ImagePtY - m_dDispImagePartRow1);
			Col0 = ImagePtX - 1 / (1 - Scale)*(ImagePtX - m_dDispImagePartCol0);
			Col1 = ImagePtX - 1 / (1 - Scale)*(ImagePtX - m_dDispImagePartCol1);

			m_dDispImagePartRow0 = Row0;
			m_dDispImagePartCol0 = Col0;
			m_dDispImagePartRow1 = Row1;
			m_dDispImagePartCol1 = Col1;
		}
		else
		{
			ImagePtX = m_dDispImagePartCol0 + (pt.x - PicRect.left) / (PicRect.Width() - 1.0)*(m_dDispImagePartCol1 - m_dDispImagePartCol0);
			ImagePtY = m_dDispImagePartRow0 + (pt.y - PicRect.top) / (PicRect.Height() - 1.0)*(m_dDispImagePartRow1 - m_dDispImagePartRow0);
			Row0 = ImagePtY - 1 / (1 + Scale)*(ImagePtY - m_dDispImagePartRow0);
			Row1 = ImagePtY - 1 / (1 + Scale)*(ImagePtY - m_dDispImagePartRow1);
			Col0 = ImagePtX - 1 / (1 + Scale)*(ImagePtX - m_dDispImagePartCol0);
			Col1 = ImagePtX - 1 / (1 + Scale)*(ImagePtX - m_dDispImagePartCol1);

			m_dDispImagePartRow0 = Row0;
			m_dDispImagePartCol0 = Col0;
			m_dDispImagePartRow1 = Row1;
			m_dDispImagePartCol1 = Col1;
		}
		ShowImage();
	}

	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);*/

	// TODO:  在此添加消息处理程序代码和/或调用默认值
	// 此功能要求 Windows Vista 或更高版本。

	// _WIN32_WINNT 符号必须 >= 0x0600。

	// TODO: 在此添加消息处理程序代码和/或调用默认值

	double dAddScal = 0.05;
	if (zDelta>0)
	{
		if (fabs(m_dScale + dAddScal - 0.5)>1e-6)
		{
			m_dScale += dAddScal;
			DisPlayImage(hoImage, hvWindowID);
			DispObj(ho_ModelTrans, HDevWindowStack::GetActive());
		}
	}
	else
	{
		if (fabs(m_dScale - dAddScal + 2)>1e-6)
		{
			m_dScale -= dAddScal;
			DisPlayImage(hoImage, hvWindowID);
			DispObj(ho_ModelTrans, HDevWindowStack::GetActive());
		}
	}

	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}


void MultiRegions::OnCbnEditupdateComboXldadded()
{
	// TODO:  在此添加控件通知处理程序代码

	int nIndex = m_combo_xldOut.GetCurSel();
	if (nIndex >= 0)
	{
		hv_single_XldList.erase(hv_single_XldList.begin() + nIndex);
		std::vector<HObject>::iterator it = hv_singleRegionList.begin() + nIndex;
		hv_singleRegionList.erase(it);

		string strTemp, strRangingName;
		strRangingName = "XLD";
		m_combo_xldOut.ResetContent();//消除现有所有内容
		for (int i = 0; i < hv_single_XldList.size(); i++)
		{
			strTemp = strRangingName + to_string(i+1);
			m_combo_xldOut.InsertString(-1, strTemp.c_str());
		}
		m_combo_xldOut.SetCurSel(hv_single_XldList.size() - 1);

		//更新list_xldIn
		m_list_xldIn.DeleteAllItems();
		for (int i = 0; i < hv_single_XldList.size(); i++)
		{
			CString s1;
			s1.Format(_T("XLD%d"), i + 1);
			m_list_xldIn.InsertItem(i, s1);
		}

		//更新m_list_detectAllRegion
		m_list_detectAllRegion.DeleteItem(nIndex);
		for (int i = nIndex; i < m_list_detectAllRegion.GetItemCount(); i++)
		{
			int no = atoi(m_list_detectAllRegion.GetItemText(i, 0));
			CString newNo = "";
			newNo.Format("%d", no - 1);
			m_list_detectAllRegion.SetItemText(i, 0, newNo);
			m_list_detectAllRegion.SetItemText(i, 3, newNo);

			CString numxldin = m_list_detectAllRegion.GetItemText(i, 4);
			CStringArray regionIn; regionIn.SetSize(50);
			int n = SplitCString(numxldin, ' ', regionIn);
			String newChildren = "";
			for (int j = 0; j < n - 1; j++)
			{
				if (nIndex != atoi(regionIn.GetAt(j)))
				{
					if (newChildren.length() > 0)
					{
						newChildren += " ";
					}
					newChildren += regionIn.GetAt(j);
				}
			}

		}

		//更新图像
		ShowImage();
	}
}


void MultiRegions::OnCbnSelchangeComboRegioninout()
{
	OnBnClickedRegionRefresh();
}


void MultiRegions::OnCbnSelchangeComboDetectmethod()
{
	OnBnClickedRegionRefresh();
}


void MultiRegions::OnCbnSelchangeComboLightdark()
{
	OnBnClickedRegionRefresh();
}


void MultiRegions::OnCbnSelchangeComboXldadded()
{
	int xldOutIndex = m_combo_xldOut.GetCurSel();
	if (xldOutIndex < 0)
	{
		return;
	}

	//刷新画面表示元素
	CString algorithm = m_list_detectAllRegion.GetItemText(xldOutIndex, 1);
	CString inOut = m_list_detectAllRegion.GetItemText(xldOutIndex, 2);
	CString xld = m_list_detectAllRegion.GetItemText(xldOutIndex, 4);
	CString mode = m_list_detectAllRegion.GetItemText(xldOutIndex, 5);

	//1.检测算法
	m_combo_detectmethod.SetCurSel(0);
	int index = m_combo_detectmethod.FindStringExact(0, algorithm);
	if (CB_ERR != index)
	{
		m_combo_detectmethod.SetCurSel(index);
	}

	//2.轮廓内外
	m_combo_lightdark.SetCurSel(0);
	index = m_combo_lightdark.FindStringExact(0, inOut);
	if (CB_ERR != index)
	{
		m_combo_lightdark.SetCurSel(index);
	}

	//5.区域模式
	m_combo_regioninout.SetCurSel(0);
	index = m_combo_regioninout.FindStringExact(0, mode);
	if (CB_ERR != index)
	{
		m_combo_regioninout.SetCurSel(index);
	}

	//4.内部轮廓列表
	//reset检查框
	for (int i = 0; i < b_reduceList.size(); i++)
	{
		m_list_xldIn.SetCheck(i, 0);
	}

	int pos = 0;
	int prePos = 0;
	if (xld.GetLength() > 0)
	{
		while (-1 != pos)
		{
			prePos = pos;
			pos = xld.Find(" ", (pos + 1));
			if (pos > prePos)
			{
				index = atoi(xld.Mid(prePos, (pos - prePos)).GetBuffer());
				if (index >= 1)
				{
					//检查框勾选
					m_list_xldIn.SetCheck(index - 1, 1);
				}
			}
		}

	}
}

double x_base;
double y_base;
double angle = 0;
double dpi = 3.38;
BOOL MultiRegions::PreTranslateMessage(MSG* pMsg)
{
	//if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP)
	//{
	//	CRect  imgRect;
	//	CPoint point;
	//	GetCursorPos(&point);
	//	CWnd * phWnd = GetDlgItem(IDC_PIC_MULTIREGION);
	//	phWnd->GetClientRect(imgRect);
	//	phWnd->ClientToScreen(imgRect);
	//	point.x = point.x - imgRect.left;
	//	point.y = point.y - imgRect.top;
	//	phWnd->GetClientRect(imgRect);
	//	if (imgRect.PtInRect(point))    //鼠标移动在窗口内
	//	{
	//		CPoint point2;
	//		point2 = point;
	//		//PicControlToImage2(point2, m_pHGMachineVisionDoc->m_iImageWidth, m_pHGMachineVisionDoc->m_iImageHeight);
	//		if (pMsg->message == WM_LBUTTONDOWN)    //鼠标左键单击
	//		{
	//			if (m_isDrawing)
	//			{
	//				//Drawing();
	//			}

	//			m_startX = point2.x;
	//			m_startY = point2.y;
	//			if (!m_isDrawing)
	//			{
	//				m_bImgMove = true;
	//			}

	//			//if (m_isDrawing)
	//			//{
	//			//	m_operationType = OPER_DRAWING;
	//			//}
	//			//else
	//			//{
	//			//	m_operationType = OPER_NONE;
	//			//}
	//		}
	//		else if (pMsg->message == WM_LBUTTONUP)
	//		{
	//			m_bImgMove = false;

	//			MoveImage(point2);

	//			m_startX = 0;
	//			m_startY = 0;

	//			//if (m_isDrawing)
	//			//{
	//			//	m_operationType = OPER_NONE;
	//			//}
	//		}
	//	}
	//}

	//if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE;
	//if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE;
	//else return CDialog::PreTranslateMessage(pMsg);
	//if (pMsg->hwnd == m_combo_xldOut.m_hWnd && pMsg->message == WM_MOUSEWHEEL) return TRUE;

	POINT ptTmp;
	GetCursorPos(&ptTmp);//获取鼠标位置 
	CRect Rect1;
	CRect rcPIC;
	CWnd *pPICWnd = GetDlgItem(IDC_PIC_MULTIREGION);
	CDC *pDCPIC = pPICWnd->GetDC();
	memset(Rect1, 0, sizeof(CRect));
	pPICWnd->ClientToScreen(Rect1);
	
	//UpdateData(TRUE);
	//坐标转换
	ptTmp.x = ptTmp.x - Rect1.left;
	ptTmp.y = ptTmp.y - Rect1.top;
	pPICWnd = GetDlgItem(IDC_PIC_MULTIREGION);
	pPICWnd->GetClientRect(&rcPIC);
	//SetColor(hvWindowID, "white");
	//判断鼠标是否在图像控件内 
	if (rcPIC.PtInRect(ptTmp))
	{
		if (pMsg->message == WM_LBUTTONDOWN)
		{
			m_ptStart = ptTmp;
			m_ptEnd = ptTmp;
			//鼠标单击事件代码 
			//HDevWindowStack::Push(hvWindowID);
			//if (HDevWindowStack::IsOpen())
			//{
			//	/*while (true)
			//	{*/
			//		/*GetMposition(hvWindowID, &hv_mouseR, &hv_mouseC, &hv_Button);
			//		cout << "R:" << hv_mouseR.I() << endl;
			//		cout << "C:" << hv_mouseC.I() << endl;*/
			//	DispObj(hoImage, HDevWindowStack::GetActive());
			//	dxf_scale(&ho_Contours, 1/dpi, &ho_ContoursScaled, &x_base, &y_base);
			//	dxf_transform(&ho_ContoursScaled, ptTmp.x, ptTmp.y, angle, &ho_ModelTrans);
			//	DispObj(ho_ModelTrans, HDevWindowStack::GetActive());
			//	//}
			//}
		}
		else if (pMsg->message == WM_LBUTTONUP)
		{
			m_ptEnd = ptTmp;
			m_nWidthOffset += (m_ptEnd.x - m_ptStart.x);
			m_nHeightOffset += (m_ptEnd.y - m_ptStart.y);
			DisPlayImage(hoImage, hvWindowID);
			DispObj(ho_ModelTrans, HDevWindowStack::GetActive());
		}
		//else if (pMsg->wParam == 'Q')
		//{
		//	//鼠标双击事件代码 
		//	HDevWindowStack::Push(hvWindowID);
		//	if (HDevWindowStack::IsOpen())
		//	{
		//		/*while (true)
		//		{*/
		//			/*GetMposition(hvWindowID, &hv_mouseR, &hv_mouseC, &hv_Button);
		//			cout << "R:" << hv_mouseR.I() << endl;
		//			cout << "C:" << hv_mouseC.I() << endl;*/
		//		DispObj(hoImage, HDevWindowStack::GetActive());
		//		dxf_scale(&ho_Contours, 1/dpi, &ho_ContoursScaled, &x_base, &y_base);
		//		dxf_transform(&ho_ContoursScaled, ptTmp.x, ptTmp.y, angle, &ho_ModelTrans);
		//		DispObj(ho_ModelTrans, HDevWindowStack::GetActive());
		//		//}
		//	}
		//}
		/*else if (pMsg->message == WM_KEYDOWN)
		{*/
		else if (pMsg->wParam == 'A')            //dxf逆时针旋转
		{
			DispObj(hoImage, HDevWindowStack::GetActive());
			dxf_scale(&ho_Contours, 1 / dpi, &ho_ContoursScaled, &x_base, &y_base);
			angle+=0.5;
			dxf_transform(&ho_ContoursScaled, ptTmp.x * m_hWidth.I() / Rect.Width(), ptTmp.y * m_hHeight.I() / Rect.Height(), angle, &ho_ModelTrans);
			DispObj(ho_ModelTrans, HDevWindowStack::GetActive());
		}
		else if (pMsg->wParam == 'D')            //dxf顺时针旋转
		{
			DispObj(hoImage, HDevWindowStack::GetActive());
			dxf_scale(&ho_Contours, 1 / dpi, &ho_ContoursScaled, &x_base, &y_base);
			angle-=0.5;
			dxf_transform(&ho_ContoursScaled, ptTmp.x * m_hWidth.I() / Rect.Width(), ptTmp.y * m_hHeight.I() / Rect.Height(), angle, &ho_ModelTrans);
			DispObj(ho_ModelTrans, HDevWindowStack::GetActive());
		}
		else if (pMsg->wParam == 'W')            //dxf放大
		{
			DispObj(hoImage, HDevWindowStack::GetActive());
			dpi += 0.1;
			dxf_scale(&ho_Contours, 1 / dpi, &ho_ContoursScaled, &x_base, &y_base);
			dxf_transform(&ho_ContoursScaled, ptTmp.x * m_hWidth.I() / Rect.Width(), ptTmp.y * m_hHeight.I() / Rect.Height(), angle, &ho_ModelTrans);
			DispObj(ho_ModelTrans, HDevWindowStack::GetActive());
		}
		else if (pMsg->wParam == 'S')            //dxf缩小
		{
			DispObj(hoImage, HDevWindowStack::GetActive());
			dpi -= 0.1;
			dxf_scale(&ho_Contours, 1 / dpi, &ho_ContoursScaled, &x_base, &y_base);
			dxf_transform(&ho_ContoursScaled, ptTmp.x * m_hWidth.I() / Rect.Width(), ptTmp.y * m_hHeight.I() / Rect.Height(), angle, &ho_ModelTrans);
			DispObj(ho_ModelTrans, HDevWindowStack::GetActive());
		}
		else if (pMsg->wParam == VK_SPACE)            //dxf平移
		{
			//HObject  ho_ContourCross, ho_PolygonsCross;
			DispObj(hoImage, HDevWindowStack::GetActive());
			dxf_scale(&ho_Contours, 1 / dpi, &ho_ContoursScaled, &x_base, &y_base);
			dxf_transform(&ho_ContoursScaled, ptTmp.x * m_hWidth.I() / Rect.Width(), ptTmp.y * m_hHeight.I() / Rect.Height(), angle, &ho_ModelTrans);
			//GenCrossContourXld(&ho_ContourCross, mousey, mousex, 88, 0);
			//GenPolygonsXld(ho_ContourCross, &ho_PolygonsCross, "ramer", 2);
			//if (HDevWindowStack::IsOpen())
			//DispObj(ho_PolygonsCross, HDevWindowStack::GetActive());
			DispObj(ho_ModelTrans, HDevWindowStack::GetActive());
			return TRUE;
		}
		/*else if (pMsg->wParam == VK_RETURN)
		{
			WriteContourXldDxf(ho_ModelTrans, "C:\\Users\\AI-009\\Desktop\\result.dxf");
			return TRUE;
		}*/
	}
	//UpdateData(FALSE);
	return CDialogEx::PreTranslateMessage(pMsg);
}

void MultiRegions::MoveImage(CPoint point/*, HImage	srcImg, HTuple hWindow*/)
{
	double xOffset = point.x - m_startX;
	double yOffset = point.y - m_startY;

	m_offsetX = m_offsetX + point.x - m_startX;
	m_offsetY = m_offsetY + point.y - m_startY;

	m_dDispImagePartRow0 = m_dDispImagePartRow0 - yOffset;
	m_dDispImagePartCol0 = m_dDispImagePartCol0 - xOffset;
	m_dDispImagePartRow1 = m_dDispImagePartRow1 - yOffset;
	m_dDispImagePartCol1 = m_dDispImagePartCol1 - xOffset;

	ShowImage();

	//ClearWindow(hWindow);
	//SetPart(hWindow, m_dDispImagePartRow0, m_dDispImagePartCol0, m_dDispImagePartRow1, m_dDispImagePartCol1);
	//DispImage(srcImg, hWindow);
}

void MultiRegions::OnCbnSelchangeComboPlottype()    //错误
{
	m_isTypeChanged = true;
}


void MultiRegions::OnBnClickedButton1()
{
	// TODO:  在此添加控件通知处理程序代码
	//HTuple hv_DxfStatus;
	//HTuple hv_ImagePath = "C:\\Users\\AI-009\\Desktop\\backbattery.bmp";
	//m_dScale = 0;//重置
	//m_nHeightOffset = 0;
	//m_nWidthOffset = 0;
	//ReadImage(&hoImage, hv_ImagePath);
	//GetImageSize(hoImage, &m_hWidth, &m_hHeight);
	//SetPart(hvWindowID, 0, 0, m_hHeight - 1, m_hWidth - 1);
	//ReadContourXldDxf(&ho_Contours, "C:\\Users\\AI-009\\Desktop\\result.dxf", HTuple(),
	//	HTuple(), &hv_DxfStatus);
	//HDevWindowStack::Push(hvWindowID);
	//if (HDevWindowStack::IsOpen())
	//{
	//	DispObj(hoImage, HDevWindowStack::GetActive());
	//	DispObj(ho_Contours, HDevWindowStack::GetActive());
	//}

	HTuple hv_Files, hv_DxfStatus, hv_ReadPath;
	CString str, target;
	//BROWSEINFO bi;
	//char name[MAX_PATH];
	//ZeroMemory(&bi, sizeof(BROWSEINFO));
	//LPITEMIDLIST rootLoation = ParsePidlFromPath(".\\location");
	//bi.hwndOwner = GetSafeHwnd();
	//bi.pszDisplayName = name;
	////bi.lpszTitle = "S浏览文件夹";
	//bi.ulFlags = BIF_RETURNONLYFSDIRS;

	//ZeroMemory(&bi, sizeof(bi));
	//bi.pidlRoot = rootLoation;
	//bi.lpszTitle = _T(""); // 可以不指定
	//LPITEMIDLIST targetLocation = SHBrowseForFolder(&bi);
	//if (targetLocation != NULL) {
	//	SHGetPathFromIDList(targetLocation, (LPSTR)(LPCSTR)target);
	//}
	//else
	//	return;
	TCHAR szBuffer[MAX_PATH] = { 0 };
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = NULL;
	bi.pszDisplayName = szBuffer;
	bi.lpszTitle = _T("从下面选择文件或文件夹:");
	bi.ulFlags = BIF_BROWSEINCLUDEFILES;
	LPITEMIDLIST idl = SHBrowseForFolder(&bi);
	if (NULL == idl)
	{
		return;
	}
	SHGetPathFromIDList(idl, szBuffer);
	//hv_ReadPath = HTuple(target);
	//ListFiles(hv_ReadPath, "files", &hv_Files);
	ReadContourXldDxf(&ho_Contours, HTuple(szBuffer), HTuple(),
			HTuple(), &hv_DxfStatus);
	HDevWindowStack::Push(hvWindowID);
	if (HDevWindowStack::IsOpen())
	{
		//DispObj(hoImage, HDevWindowStack::GetActive());
		DispObj(ho_Contours, HDevWindowStack::GetActive());
	}
}

void dev_update_off()
{
	return;
}
void dev_update_on()
{
	return;
}

//void genRegionsbyRegionGrowing(HObject ho_Contours, HObject* ho_GenRegions, HObject* ho_GenContours,
//	HTuple hv_Width, HTuple hv_Height, HTuple* hv_RegionNumber)
//{
//
//	// Local iconic variables
//	HObject  ho_ContoursImage;
//
//	// Local control variables
//	HTuple  hv_SaveImageWindowID;
//
//	SetWindowAttr("background_color", "white");
//	OpenWindow(0, 0, hv_Width, hv_Height, 0, "", "", &hv_SaveImageWindowID);
//	HDevWindowStack::Push(hv_SaveImageWindowID);
//	//dev_display (Image)
//	if (HDevWindowStack::IsOpen())
//		SetColor(HDevWindowStack::GetActive(), "black");
//	if (HDevWindowStack::IsOpen())
//		DispObj(ho_Contours, HDevWindowStack::GetActive());
//	DumpWindowImage(&ho_ContoursImage, hv_SaveImageWindowID);
//	if (HDevWindowStack::IsOpen())
//		CloseWindow(HDevWindowStack::Pop());
//	RegiongrowingN(ho_ContoursImage, &(*ho_GenRegions), "dot-product", 0, 60, 30);
//	CountObj((*ho_GenRegions), &(*hv_RegionNumber));
//	SortRegion((*ho_GenRegions), &(*ho_GenRegions), "upper_left", "true", "row");
//	GenContourRegionXld((*ho_GenRegions), &(*ho_GenContours), "border");
//	return;
//}
//
//void getUserSelectedRegions(HObject ho_Image, HObject ho_GenRegions, HObject ho_GenContours,
//	HTuple hv_WindowID, HTuple* hv_contourGroupIdx)
//{
//
//	// Local iconic variables
//	HObject  ho_contourSelected, ho_selectedContours;
//	HObject  ho_currentToShow, ho_notCurrentShow, ho_selectedNewAdd;
//
//	// Local control variables
//	HTuple  hv_Error;
//	HTuple  hv_WindowIDOut, hv_RegionNumber, hv_mButton;
//	HTuple  hv_ButtonPrev, hv_current, hv_finishFlag, hv_i;
//	HTuple  hv_Area, hv_Row, hv_Column, hv_po, hv_isInside;
//	HTuple  hv_keyInput, hv_controlKey, hv_cntN, hv_numberExist;
//	HTuple  hv_index, hv_deleteIndex, hv_indexNotCurrent;
//
//	hv_WindowIDOut = hv_WindowID;
//	CountObj(ho_GenRegions, &hv_RegionNumber);
//	hv_WindowIDOut = hv_WindowIDOut;
//	hv_mButton = 0;
//	hv_ButtonPrev = 0;
//	(*hv_contourGroupIdx) = HTuple();
//	if (HDevWindowStack::IsOpen())
//		ClearWindow(HDevWindowStack::GetActive());
//	hv_current = 0;
//	hv_finishFlag = 0;
//	while (0 != (0 != 1))
//	{
//		if (0 != hv_finishFlag)
//		{
//			break;
//		}
//		dev_update_on();
//		HDevWindowStack::SetActive(hv_WindowIDOut);
//		if (HDevWindowStack::IsOpen())
//			DispObj(ho_Image, HDevWindowStack::GetActive());
//		if (0 != (hv_current != 0))
//		{
//			if (HDevWindowStack::IsOpen())
//			{
//				SetColor(HDevWindowStack::GetActive(), "black");
//				SetLineWidth(HDevWindowStack::GetActive(), 1);
//			}
//
//		}
//		//dev_display (GenRegions)
//		{
//			HTuple end_val33 = hv_RegionNumber;
//			HTuple step_val33 = 1;
//			for (hv_i = 1; hv_i.Continue(end_val33, step_val33); hv_i += step_val33)
//			{
//				SelectObj(ho_GenContours, &ho_contourSelected, hv_i);
//				//**dev_set_color(colors[i%7])
//				if (HDevWindowStack::IsOpen())
//					SetLineWidth(HDevWindowStack::GetActive(), 2);
//				if (HDevWindowStack::IsOpen())
//					SetColor(HDevWindowStack::GetActive(), "gray");
//				if (HDevWindowStack::IsOpen())
//					DispObj(ho_contourSelected, HDevWindowStack::GetActive());
//				if (HDevWindowStack::IsOpen())
//					SetColor(HDevWindowStack::GetActive(), "red");
//				AreaCenterXld(ho_contourSelected, &hv_Area, &hv_Row, &hv_Column, &hv_po);
//				SetTposition(hv_WindowIDOut, hv_Row, hv_Column);
//				WriteString(hv_WindowIDOut, hv_i);
//				if (HDevWindowStack::IsOpen()) {
//					SetLineWidth(HDevWindowStack::GetActive(), 1);
//					SetColor(HDevWindowStack::GetActive(), "black");
//				}
//			}
//		}
//		if (0 != ((*hv_contourGroupIdx) != HTuple()))
//		{
//			SelectObj(ho_GenContours, &ho_selectedContours, (*hv_contourGroupIdx));
//			if (HDevWindowStack::IsOpen())
//			{
//				SetColor(HDevWindowStack::GetActive(), "green");
//				SetLineWidth(HDevWindowStack::GetActive(), 3);
//				DispObj(ho_selectedContours, HDevWindowStack::GetActive());
//				SetLineWidth(HDevWindowStack::GetActive(), 1);
//				SetColor(HDevWindowStack::GetActive(), "black");
//			}
//		}
//		if (HDevWindowStack::IsOpen())
//		{
//			SetLineWidth(HDevWindowStack::GetActive(), 1);
//			SetColor(HDevWindowStack::GetActive(), "black");
//		}
//		dev_update_off();
//		// Error variable 'hv_Error' activated
//		hv_Error = 2;
//		// dev_set_check ("~give_error")
//		//*** get_mbutton(WindowID, Row, Column, mButton)
//		try
//		{
//			hv_Error = 2;
//			GetMposition(hv_WindowIDOut, &hv_Row, &hv_Column, &hv_mButton);
//		}
//		catch (HalconCpp::HException e)
//		{
//			hv_Error = (int)e.ErrorCode();
//			if (hv_Error < 0)
//				throw e;
//		}
//
//		// Error variable 'hv_Error' deactivated
//		// dev_set_check ("give_error")
//		if (0 != (hv_Error == 2))
//		{
//		}
//		//*** TODO：一次只添加一个
//		hv_current = 0;
//		{
//			HTuple end_val73 = hv_RegionNumber;
//			HTuple step_val73 = 1;
//			for (hv_i = 1; hv_i.Continue(end_val73, step_val73); hv_i += step_val73)
//			{
//				hv_isInside = 0;
//				SelectObj(ho_GenContours, &ho_contourSelected, hv_i);
//				TestXldPoint(ho_contourSelected, hv_Row, hv_Column, &hv_isInside);
//				if (0 != (hv_isInside == 1))
//				{
//					if (0 != (hv_i > hv_current))
//					{
//						hv_current = hv_i;
//					}
//				}
//			}
//		}
//		if (0 != (hv_mButton != 0))
//		{
//			if (0 != (hv_mButton == 1))
//			{
//				if (0 != (hv_current != 0))
//				{
//					//*** 显示提示信息--start
//					if (HDevWindowStack::IsOpen())
//						SetColor(HDevWindowStack::GetActive(), "black");
//					//*** 显示提示信息--end
//					ReadChar(hv_WindowIDOut, &hv_keyInput, &hv_controlKey);
//					if (0 != (hv_keyInput == HTuple("3")))
//					{
//						//***  cancel
//						(*hv_contourGroupIdx) = HTuple();
//					}
//					if (0 != (hv_keyInput == HTuple("1")))
//					{
//						//*** add
//						TupleLength((*hv_contourGroupIdx), &hv_cntN);
//						if (0 != (hv_cntN == 0))
//						{
//							(*hv_contourGroupIdx) = (*hv_contourGroupIdx).TupleConcat(hv_current);
//						}
//						else
//						{
//							hv_numberExist = 0;
//							{
//								HTuple end_val117 = hv_cntN;
//								HTuple step_val117 = 1;
//								for (hv_index = 1; hv_index.Continue(end_val117, step_val117); hv_index += step_val117)
//								{
//									if (0 != (HTuple((*hv_contourGroupIdx)[hv_index - 1]) == hv_current))
//									{
//										hv_numberExist = 1;
//									}
//								}
//							}
//							if (0 != (hv_numberExist == 0))
//							{
//								(*hv_contourGroupIdx) = (*hv_contourGroupIdx).TupleConcat(hv_current);
//							}
//						}
//					}
//					if (0 != (hv_keyInput == HTuple("2")))
//					{
//						//*** delete
//						TupleLength((*hv_contourGroupIdx), &hv_cntN);
//						if (0 != (hv_cntN == 0))
//						{
//							//* TODO： output warning： 只能删除当前已经选择的区域
//						}
//						else
//						{
//							hv_numberExist = 0;
//							{
//								HTuple end_val134 = hv_cntN;
//								HTuple step_val134 = 1;
//								for (hv_index = 1; hv_index.Continue(end_val134, step_val134); hv_index += step_val134)
//								{
//									if (0 != (HTuple((*hv_contourGroupIdx)[hv_index - 1]) == hv_current))
//									{
//										hv_numberExist = 1;
//										hv_deleteIndex = hv_index - 1;
//									}
//								}
//							}
//							if (0 != (hv_numberExist == 1))
//							{
//								TupleRemove((*hv_contourGroupIdx), hv_deleteIndex, &(*hv_contourGroupIdx));
//							}
//						}
//					}
//					if (0 != (hv_keyInput == HTuple("5")))
//					{
//						//*** finish
//						hv_finishFlag = 1;
//					}
//				}
//			}
//		}
//		dev_update_on();
//		if (0 != (hv_current != 0))
//		{
//			SelectObj(ho_GenContours, &ho_currentToShow, hv_current);
//			if (HDevWindowStack::IsOpen())
//			{
//				SetColor(HDevWindowStack::GetActive(), "red");
//				SetLineWidth(HDevWindowStack::GetActive(), 2);
//				DispObj(ho_currentToShow, HDevWindowStack::GetActive());
//				SetLineWidth(HDevWindowStack::GetActive(), 1);
//				SetColor(HDevWindowStack::GetActive(), "black");
//				SetLineWidth(HDevWindowStack::GetActive(), 1);
//			}
//		}
//
//		if (HDevWindowStack::IsOpen())
//
//		{
//			HTuple end_val162 = hv_RegionNumber;
//			HTuple step_val162 = 1;
//			for (hv_indexNotCurrent = 1; hv_indexNotCurrent.Continue(end_val162, step_val162); hv_indexNotCurrent += step_val162)
//			{
//				if (0 != (hv_indexNotCurrent != hv_current))
//				{
//					if (HDevWindowStack::IsOpen())
//						SetColor(HDevWindowStack::GetActive(), "gray");
//					if (HDevWindowStack::IsOpen())
//						SetLineWidth(HDevWindowStack::GetActive(), 2);
//					SelectObj(ho_GenContours, &ho_notCurrentShow, hv_indexNotCurrent);
//					if (HDevWindowStack::IsOpen())
//						DispObj(ho_notCurrentShow, HDevWindowStack::GetActive());
//				}
//			}
//		}
//		if (0 != ((*hv_contourGroupIdx) != HTuple()))
//		{
//			SelectObj(ho_GenContours, &ho_selectedNewAdd, (*hv_contourGroupIdx));
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "green");
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 3);
//			if (HDevWindowStack::IsOpen())
//				DispObj(ho_selectedNewAdd, HDevWindowStack::GetActive());
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 1);
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "black");
//		}
//		dev_update_off();
//	}
//	return;
//}







//void getUserSelectedRegions(HObject ho_Image, HObject ho_GenRegions, HObject ho_GenContours,
//	HTuple hv_WindowID, HTuple hv_IsDebug, HTuple *hv_contourGroupIdx)
//{
//
//	// Local iconic variables
//	HObject  ho_cnt, ho_selectedContours, ho_contourSelected;
//	HObject  ho_currentToShow, ho_showNotCurrent, ho_selectedNewAdd;
//
//	// Local control variables
//	HTuple  hv_Error;
//	HTuple  hv_DEBUGMODE, hv_WindowIDOut, hv_RegionNumber;
//	HTuple  hv_mButton, hv_ButtonPrev, hv_current, hv_finishFlag;
//	HTuple  hv_i, hv_Area, hv_Row, hv_Column, hv_po, hv_isInside;
//	HTuple  hv_keyInput, hv_controlKey, hv_cntN, hv_numberExist;
//	HTuple  hv_index, hv_deleteIndex, hv_indexNotCurrent;
//
//	hv_DEBUGMODE = hv_IsDebug;
//	hv_WindowIDOut = hv_WindowID;
//	CountObj(ho_GenRegions, &hv_RegionNumber);
//	hv_WindowIDOut = hv_WindowIDOut;
//	hv_mButton = 0;
//	hv_ButtonPrev = 0;
//	(*hv_contourGroupIdx) = HTuple();
//	if (HDevWindowStack::IsOpen())
//		ClearWindow(HDevWindowStack::GetActive());
//	hv_current = 0;
//	hv_finishFlag = 0;
//	while (0 != (0 != 1))
//	{
//		dev_update_on();
//		HDevWindowStack::SetActive(hv_WindowIDOut);
//		if (HDevWindowStack::IsOpen())
//			DispObj(ho_Image, HDevWindowStack::GetActive());
//		if (0 != hv_finishFlag)
//		{
//			if (0 != (hv_DEBUGMODE == 1))
//			{
//				SetTposition(hv_WindowIDOut, 10, 150);
//				if (HDevWindowStack::IsOpen())
//					SetColor(HDevWindowStack::GetActive(), "yellow");
//				WriteString(hv_WindowIDOut, "用户选择的输入为:");
//				WriteString(hv_WindowIDOut, " R-" + (*hv_contourGroupIdx));
//			}
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "black");
//			break;
//		}
//		if (0 != (HTuple(hv_current != 0).TupleAnd(hv_DEBUGMODE)))
//		{
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "black");
//			SetTposition(hv_WindowIDOut, 10, 50);
//			DispRectangle1(hv_WindowIDOut, 10, 50, 250, 1000);
//			SetTposition(hv_WindowIDOut, 10, 150);
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 2);
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "red");
//			WriteString(hv_WindowIDOut, "当前所在区域:" + hv_current);
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 1);
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "black");
//		}
//		{
//			HTuple end_val35 = hv_RegionNumber;
//			HTuple step_val35 = 1;
//			for (hv_i = 1; hv_i.Continue(end_val35, step_val35); hv_i += step_val35)
//			{
//				SelectObj(ho_GenContours, &ho_cnt, hv_i);
//				//**dev_set_color(colors[i%7])
//				if (HDevWindowStack::IsOpen())
//					SetLineWidth(HDevWindowStack::GetActive(), 2);
//				if (HDevWindowStack::IsOpen())
//					SetColor(HDevWindowStack::GetActive(), "gray");
//				if (HDevWindowStack::IsOpen())
//					DispObj(ho_cnt, HDevWindowStack::GetActive());
//				if (HDevWindowStack::IsOpen())
//					SetColor(HDevWindowStack::GetActive(), "red");
//				AreaCenterXld(ho_cnt, &hv_Area, &hv_Row, &hv_Column, &hv_po);
//				SetTposition(hv_WindowIDOut, hv_Row, hv_Column);
//				WriteString(hv_WindowIDOut, "ROI-" + hv_i);
//				if (HDevWindowStack::IsOpen())
//					SetLineWidth(HDevWindowStack::GetActive(), 1);
//				if (HDevWindowStack::IsOpen())
//					SetColor(HDevWindowStack::GetActive(), "black");
//			}
//		}
//		if (0 != ((*hv_contourGroupIdx) != HTuple()))
//		{
//			SelectObj(ho_GenContours, &ho_selectedContours, (*hv_contourGroupIdx));
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "green");
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 3);
//			if (HDevWindowStack::IsOpen())
//				DispObj(ho_selectedContours, HDevWindowStack::GetActive());
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 1);
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "black");
//		}
//		if (HDevWindowStack::IsOpen())
//			SetLineWidth(HDevWindowStack::GetActive(), 1);
//		if (HDevWindowStack::IsOpen())
//			SetColor(HDevWindowStack::GetActive(), "black");
//		dev_update_off();
//		//*********************************************************************
//		//*********************************************************************
//		// Error variable 'hv_Error' activated
//		hv_Error = 2;
//		// dev_set_check ("~give_error")
//		//*********************************************************************
//		//*********************************************************************
//		//*** get_mbutton(WindowID, Row, Column, mButton)
//		try
//		{
//			hv_Error = 2;
//			GetMposition(hv_WindowIDOut, &hv_Row, &hv_Column, &hv_mButton);
//		}
//		catch (HalconCpp::HException e)
//		{
//			hv_Error = (int)e.ErrorCode();
//			if (hv_Error < 0)
//				throw e;
//		}
//		//*********************************************************************
//		//*********************************************************************
//		// Error variable 'hv_Error' deactivated
//		// dev_set_check ("give_error")
//		if (0 != (hv_Error == 2))
//		{
//		}
//		//*** TODO：一次只添加一个
//		hv_current = 0;
//		{
//			HTuple end_val75 = hv_RegionNumber;
//			HTuple step_val75 = 1;
//			for (hv_i = 1; hv_i.Continue(end_val75, step_val75); hv_i += step_val75)
//			{
//				hv_isInside = 0;
//				SelectObj(ho_GenContours, &ho_contourSelected, hv_i);
//				TestXldPoint(ho_contourSelected, hv_Row, hv_Column, &hv_isInside);
//				if (0 != (hv_isInside == 1))
//				{
//					if (0 != (hv_i>hv_current))
//					{
//						hv_current = hv_i;
//					}
//				}
//			}
//		}
//		if (0 != (hv_mButton != 0))
//		{
//			if (0 != (hv_mButton == 1))
//			{
//				if (0 != (hv_current != 0))
//				{
//					if (0 != hv_DEBUGMODE)
//					{
//						//*** 显示提示信息--start
//						if (HDevWindowStack::IsOpen())
//							SetColor(HDevWindowStack::GetActive(), "black");
//						SetTposition(hv_WindowIDOut, 10, 50);
//						DispRectangle1(hv_WindowIDOut, 10, 50, 50, 100);
//						SetTposition(hv_WindowIDOut, 10, 50);
//						if (HDevWindowStack::IsOpen())
//							SetColor(HDevWindowStack::GetActive(), "green");
//						WriteString(hv_WindowIDOut, "请按键:");
//						SetTposition(hv_WindowIDOut, 50, 70);
//						WriteString(hv_WindowIDOut, "1：选择/添加一个区域");
//						SetTposition(hv_WindowIDOut, 80, 70);
//						WriteString(hv_WindowIDOut, "2：删除当前选择的某个区域");
//						SetTposition(hv_WindowIDOut, 110, 70);
//						WriteString(hv_WindowIDOut, "3：清空当前已选择区域");
//						SetTposition(hv_WindowIDOut, 140, 70);
//						WriteString(hv_WindowIDOut, "4：取消当前状态，继续选择下一个区域");
//						SetTposition(hv_WindowIDOut, 170, 70);
//						WriteString(hv_WindowIDOut, "5：完成选择过程，返回结果并退出");
//						if (HDevWindowStack::IsOpen())
//							SetColor(HDevWindowStack::GetActive(), "black");
//						//*** 显示提示信息--end
//					}
//					ReadChar(hv_WindowIDOut, &hv_keyInput, &hv_controlKey);
//					if (0 != (hv_keyInput == HTuple("3")))
//					{
//						//***  cancel
//						(*hv_contourGroupIdx) = HTuple();
//					}
//					if (0 != (hv_keyInput == HTuple("1")))
//					{
//						//*** add
//						TupleLength((*hv_contourGroupIdx), &hv_cntN);
//						if (0 != (hv_cntN == 0))
//						{
//							(*hv_contourGroupIdx) = (*hv_contourGroupIdx).TupleConcat(hv_current);
//						}
//						else
//						{
//							hv_numberExist = 0;
//							{
//								HTuple end_val121 = hv_cntN;
//								HTuple step_val121 = 1;
//								for (hv_index = 1; hv_index.Continue(end_val121, step_val121); hv_index += step_val121)
//								{
//									if (0 != (HTuple((*hv_contourGroupIdx)[hv_index - 1]) == hv_current))
//									{
//										hv_numberExist = 1;
//									}
//								}
//							}
//							if (0 != (hv_numberExist == 0))
//							{
//								(*hv_contourGroupIdx) = (*hv_contourGroupIdx).TupleConcat(hv_current);
//							}
//						}
//					}
//					if (0 != (hv_keyInput == HTuple("2")))
//					{
//						//*** delete
//						TupleLength((*hv_contourGroupIdx), &hv_cntN);
//						if (0 != (hv_cntN == 0))
//						{
//							//* TODO： output warning： 只能删除当前已经选择的区域
//						}
//						else
//						{
//							hv_numberExist = 0;
//							{
//								HTuple end_val138 = hv_cntN;
//								HTuple step_val138 = 1;
//								for (hv_index = 1; hv_index.Continue(end_val138, step_val138); hv_index += step_val138)
//								{
//									if (0 != (HTuple((*hv_contourGroupIdx)[hv_index - 1]) == hv_current))
//									{
//										hv_numberExist = 1;
//										hv_deleteIndex = hv_index - 1;
//									}
//								}
//							}
//							if (0 != (hv_numberExist == 1))
//							{
//								TupleRemove((*hv_contourGroupIdx), hv_deleteIndex, &(*hv_contourGroupIdx));
//							}
//						}
//					}
//					if (0 != (hv_keyInput == HTuple("5")))
//					{
//						//*** finish
//						if (0 != hv_DEBUGMODE)
//						{
//							//***
//						}
//						hv_finishFlag = 1;
//					}
//				}
//			}
//		}
//		dev_update_on();
//		if (0 != (hv_current != 0))
//		{
//			SelectObj(ho_GenContours, &ho_currentToShow, hv_current);
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "red");
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 2);
//			if (HDevWindowStack::IsOpen())
//				DispObj(ho_currentToShow, HDevWindowStack::GetActive());
//		}
//		if (HDevWindowStack::IsOpen())
//			SetColor(HDevWindowStack::GetActive(), "black");
//		if (HDevWindowStack::IsOpen())
//			SetLineWidth(HDevWindowStack::GetActive(), 1);
//		{
//			HTuple end_val168 = hv_RegionNumber;
//			HTuple step_val168 = 1;
//			for (hv_indexNotCurrent = 1; hv_indexNotCurrent.Continue(end_val168, step_val168); hv_indexNotCurrent += step_val168)
//			{
//				if (0 != (hv_indexNotCurrent != hv_current))
//				{
//					if (HDevWindowStack::IsOpen())
//						SetColor(HDevWindowStack::GetActive(), "gray");
//					if (HDevWindowStack::IsOpen())
//						SetLineWidth(HDevWindowStack::GetActive(), 2);
//					SelectObj(ho_GenContours, &ho_showNotCurrent, hv_indexNotCurrent);
//					if (HDevWindowStack::IsOpen())
//						DispObj(ho_showNotCurrent, HDevWindowStack::GetActive());
//				}
//			}
//		}
//		if (0 != ((*hv_contourGroupIdx) != HTuple()))
//		{
//			SelectObj(ho_GenContours, &ho_selectedNewAdd, (*hv_contourGroupIdx));
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "green");
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 3);
//			if (HDevWindowStack::IsOpen())
//				DispObj(ho_selectedNewAdd, HDevWindowStack::GetActive());
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 1);
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "black");
//		}
//		dev_update_off();
//		//stop ()
//	}
//	return;
//}
//
//void genRegionsbyRegionGrowing(HObject ho_ImageBG, HObject ho_Contours,
//	HObject *ho_GenRegions, HObject *ho_GenContours, HTuple hv_Width, HTuple hv_Height,
//	HTuple hv_IsDebug, HTuple *hv_RegionNumber)
//{
//
//	// Local iconic variables
//	HObject  ho_ContoursOut, ho_ContoursImage, ho_Regions;
//
//	// Local control variables
//	HTuple  hv_DebugMode, hv_SaveImageWindowID, hv_thv;
//
//	hv_DebugMode = hv_IsDebug;
//	ho_ContoursOut = ho_Contours;
//	SetWindowAttr("background_color", "white");
//	OpenWindow(0, 0, hv_Width, hv_Height, 0, "", "", &hv_SaveImageWindowID);
//	HDevWindowStack::Push(hv_SaveImageWindowID);
//	ZoomImageSize(ho_ImageBG, &ho_ImageBG, hv_Width, hv_Height, "constant");
//	if (0 != hv_DebugMode)
//	{
//		if (HDevWindowStack::IsOpen())
//			DispObj(ho_ImageBG, HDevWindowStack::GetActive());
//		// stop(); only in hdevelop
//	}
//
//	UnionAdjacentContoursXld(ho_ContoursOut, &ho_ContoursOut, 2, 2, "attr_keep");
//	if (0 != hv_DebugMode)
//	{
//		if (HDevWindowStack::IsOpen())
//			SetColor(HDevWindowStack::GetActive(), "black");
//		if (HDevWindowStack::IsOpen())
//			SetLineWidth(HDevWindowStack::GetActive(), 2);
//		if (HDevWindowStack::IsOpen())
//			DispObj(ho_ContoursOut, HDevWindowStack::GetActive());
//		// stop(); only in hdevelop
//	}
//
//	DumpWindowImage(&ho_ContoursImage, hv_SaveImageWindowID);
//	ZoomImageSize(ho_ContoursImage, &ho_ContoursImage, hv_Width, hv_Height, "constant");
//
//	BinaryThreshold(ho_ContoursImage, &ho_Regions, "max_separability", "dark", &hv_thv);
//	BackgroundSeg(ho_Regions, &(*ho_GenRegions));
//
//	CountObj((*ho_GenRegions), &(*hv_RegionNumber));
//	SortRegion((*ho_GenRegions), &(*ho_GenRegions), "upper_left", "true", "row");
//	GenContourRegionXld((*ho_GenRegions), &(*ho_GenContours), "border");
//	if (0 != hv_DebugMode)
//	{
//		if (HDevWindowStack::IsOpen())
//			SetColored(HDevWindowStack::GetActive(), 12);
//		if (HDevWindowStack::IsOpen())
//			DispObj((*ho_GenRegions), HDevWindowStack::GetActive());
//		// stop(); only in hdevelop
//	}
//	return;
//}


//void get_user_selected_ROIs(HObject ho_Image, HObject ho_GenRegions, HObject ho_GenContours,
//	HTuple hv_WindowID, HTuple *hv_contourGroupIdx)
//{
//
//	// Local iconic variables
//	HObject  ho_cnt, ho_selectedContours, ho_contourSelected;
//
//	// Local control variables
//	HTuple  hv_Error;
//	HTuple  hv_WindowIDOut, hv_RegionNumber, hv_mButton;
//	HTuple  hv_ButtonPrev, hv_current, hv_finishFlag, hv_i;
//	HTuple  hv_Area, hv_Row, hv_Column, hv_po, hv_isInside;
//	HTuple  hv_keyInput, hv_controlKey, hv_cntN, hv_numberExist;
//	HTuple  hv_index, hv_deleteIndex;
//
//	hv_WindowIDOut = hv_WindowID;
//	CountObj(ho_GenRegions, &hv_RegionNumber);
//	hv_WindowIDOut = hv_WindowIDOut;
//	hv_mButton = 0;
//	hv_ButtonPrev = 0;
//	(*hv_contourGroupIdx) = HTuple();
//	if (HDevWindowStack::IsOpen())
//		ClearWindow(HDevWindowStack::GetActive());
//	hv_current = 0;
//	hv_finishFlag = 0;
//	while (0 != (0 != 1))
//	{
//		dev_update_on();
//		HDevWindowStack::SetActive(hv_WindowIDOut);
//		if (HDevWindowStack::IsOpen())
//			DispObj(ho_Image, HDevWindowStack::GetActive());
//		if (0 != hv_finishFlag)
//		{
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "black");
//			break;
//		}
//		if (HDevWindowStack::IsOpen())
//			SetLineWidth(HDevWindowStack::GetActive(), 2);
//		{
//			HTuple end_val18 = hv_RegionNumber;
//			HTuple step_val18 = 1;
//			for (hv_i = 1; hv_i.Continue(end_val18, step_val18); hv_i += step_val18)
//			{
//				SelectObj(ho_GenContours, &ho_cnt, hv_i);
//				//**dev_set_color(colors[i%7])
//				if (0 != (hv_i == hv_current))
//				{
//					if (HDevWindowStack::IsOpen())
//						SetColor(HDevWindowStack::GetActive(), "red");
//					if (HDevWindowStack::IsOpen())
//						DispObj(ho_cnt, HDevWindowStack::GetActive());
//					AreaCenterXld(ho_cnt, &hv_Area, &hv_Row, &hv_Column, &hv_po);
//					SetTposition(hv_WindowIDOut, hv_Row, hv_Column);
//					WriteString(hv_WindowIDOut, "ROI-" + hv_i);
//				}
//				else
//				{
//					if (HDevWindowStack::IsOpen())
//						SetColor(HDevWindowStack::GetActive(), "gray");
//					if (HDevWindowStack::IsOpen())
//						DispObj(ho_cnt, HDevWindowStack::GetActive());
//				}
//			}
//		}
//		if (0 != ((*hv_contourGroupIdx) != HTuple()))
//		{
//			SelectObj(ho_GenContours, &ho_selectedContours, (*hv_contourGroupIdx));
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "green");
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 3);
//			if (HDevWindowStack::IsOpen())
//				DispObj(ho_selectedContours, HDevWindowStack::GetActive());
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 1);
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "black");
//		}
//		if (HDevWindowStack::IsOpen())
//			SetLineWidth(HDevWindowStack::GetActive(), 1);
//		if (HDevWindowStack::IsOpen())
//			SetColor(HDevWindowStack::GetActive(), "black");
//		dev_update_off();
//		//*********************************************************************
//		//*********************************************************************
//		// Error variable 'hv_Error' activated
//		hv_Error = 2;
//		// dev_set_check ("~give_error")
//		//*********************************************************************
//		//*********************************************************************
//		//*** get_mbutton(WindowID, Row, Column, mButton)
//		try
//		{
//			hv_Error = 2;
//			GetMposition(hv_WindowIDOut, &hv_Row, &hv_Column, &hv_mButton);
//		}
//		catch (HalconCpp::HException e)
//		{
//			hv_Error = (int)e.ErrorCode();
//			if (hv_Error < 0)
//				throw e;
//		}
//		//*********************************************************************
//		//*********************************************************************
//		// Error variable 'hv_Error' deactivated
//		// dev_set_check ("give_error")
//		if (0 != (hv_Error == 2))
//		{
//		}
//		//*** TODO：一次只添加一个
//		hv_current = 0;
//		{
//			HTuple end_val59 = hv_RegionNumber;
//			HTuple step_val59 = 1;
//			for (hv_i = 1; hv_i.Continue(end_val59, step_val59); hv_i += step_val59)
//			{
//				hv_isInside = 0;
//				SelectObj(ho_GenContours, &ho_contourSelected, hv_i);
//				TestXldPoint(ho_contourSelected, hv_Row, hv_Column, &hv_isInside);
//				if (0 != (hv_isInside == 1))
//				{
//					if (0 != (hv_i>hv_current))
//					{
//						hv_current = hv_i;
//					}
//				}
//			}
//		}
//		if (0 != (hv_mButton != 0))
//		{
//			if (0 != (hv_mButton == 1))
//			{
//				if (0 != (hv_current != 0))
//				{
//					ReadChar(hv_WindowIDOut, &hv_keyInput, &hv_controlKey);
//					if (0 != (hv_keyInput == HTuple("3")))
//					{
//						//***  cancel
//						(*hv_contourGroupIdx) = HTuple();
//					}
//					if (0 != (hv_keyInput == HTuple("1")))
//					{
//						//*** add
//						TupleLength((*hv_contourGroupIdx), &hv_cntN);
//						if (0 != (hv_cntN == 0))
//						{
//							(*hv_contourGroupIdx) = (*hv_contourGroupIdx).TupleConcat(hv_current);
//						}
//						else
//						{
//							hv_numberExist = 0;
//							{
//								HTuple end_val84 = hv_cntN;
//								HTuple step_val84 = 1;
//								for (hv_index = 1; hv_index.Continue(end_val84, step_val84); hv_index += step_val84)
//								{
//									if (0 != (HTuple((*hv_contourGroupIdx)[hv_index - 1]) == hv_current))
//									{
//										hv_numberExist = 1;
//									}
//								}
//							}
//							if (0 != (hv_numberExist == 0))
//							{
//								(*hv_contourGroupIdx) = (*hv_contourGroupIdx).TupleConcat(hv_current);
//							}
//						}
//					}
//					if (0 != (hv_keyInput == HTuple("2")))
//					{
//						//*** delete
//						TupleLength((*hv_contourGroupIdx), &hv_cntN);
//						if (0 != (hv_cntN == 0))
//						{
//							//* TODO： output warning： 只能删除当前已经选择的区域
//						}
//						else
//						{
//							hv_numberExist = 0;
//							{
//								HTuple end_val101 = hv_cntN;
//								HTuple step_val101 = 1;
//								for (hv_index = 1; hv_index.Continue(end_val101, step_val101); hv_index += step_val101)
//								{
//									if (0 != (HTuple((*hv_contourGroupIdx)[hv_index - 1]) == hv_current))
//									{
//										hv_numberExist = 1;
//										hv_deleteIndex = hv_index - 1;
//									}
//								}
//							}
//							if (0 != (hv_numberExist == 1))
//							{
//								TupleRemove((*hv_contourGroupIdx), hv_deleteIndex, &(*hv_contourGroupIdx));
//							}
//						}
//					}
//					if (0 != (hv_keyInput == HTuple("5")))
//					{
//						//*** finish
//						hv_finishFlag = 1;
//					}
//				}
//			}
//		}
//		dev_update_on();
//	}
//	return;
//}

//void gen_regions_by_contours_bingary_image(HObject ho_ImageBG, HObject ho_Contours,
//	HObject *ho_GenRegions, HObject *ho_GenContours, HTuple hv_Width, HTuple hv_Height,
//	HTuple *hv_RegionNumber)
//{
//
//	// Local iconic variables
//	HObject  ho_ContoursOut, ho_ContoursImageWindow;
//	HObject  ho_ContoursImage, ho_Regions;
//
//	// Local control variables
//	HTuple  hv_SaveImageWindowID, hv_thv;
//
//	ho_ContoursOut = ho_Contours;
//	SetWindowAttr("background_color", "white");
//	OpenWindow(0, 0, hv_Width, hv_Height, 0, "", "", &hv_SaveImageWindowID);
//	HDevWindowStack::Push(hv_SaveImageWindowID);
//	ZoomImageSize(ho_ImageBG, &ho_ImageBG, hv_Width, hv_Height, "constant");
//	if (HDevWindowStack::IsOpen())
//		DispObj(ho_ImageBG, HDevWindowStack::GetActive());
//
//	UnionAdjacentContoursXld(ho_ContoursOut, &ho_ContoursOut, 2, 2, "attr_keep");
//	if (HDevWindowStack::IsOpen())
//		SetColor(HDevWindowStack::GetActive(), "black");
//	if (HDevWindowStack::IsOpen())
//		SetLineWidth(HDevWindowStack::GetActive(), 2);
//	if (HDevWindowStack::IsOpen())
//		DispObj(ho_ContoursOut, HDevWindowStack::GetActive());
//
//	DumpWindowImage(&ho_ContoursImageWindow, hv_SaveImageWindowID);
//	CopyImage(ho_ContoursImageWindow, &ho_ContoursImage);
//	ZoomImageSize(ho_ContoursImage, &ho_ContoursImage, hv_Width, hv_Height, "constant");
//	BinaryThreshold(ho_ContoursImage, &ho_Regions, "max_separability", "dark", &hv_thv);
//	BackgroundSeg(ho_Regions, &(*ho_GenRegions));
//
//	CountObj((*ho_GenRegions), &(*hv_RegionNumber));
//	SortRegion((*ho_GenRegions), &(*ho_GenRegions), "upper_left", "true", "row");
//	GenContourRegionXld((*ho_GenRegions), &(*ho_GenContours), "border");
//	if (HDevWindowStack::IsOpen())
//		CloseWindow(HDevWindowStack::Pop());
//	return;
//}


//void get_user_selected_ROIs(HObject ho_Image, HObject ho_GenRegions, HObject ho_GenContours,
//	HTuple hv_WindowID, HTuple *hv_regionIdDefinedByUser, HTuple *hv_selectedRegionIdList,
//	HTuple *hv_mousePositionRegionID)
//{
//
//	// Local iconic variables
//	HObject  ho_cnt, ho_selectedContours, ho_contourSelected;
//
//	// Local control variables
//	HTuple  hv_Error;
//	HTuple  hv_WindowIDOut, hv_RegionNumber, hv_index;
//	HTuple  hv_ImgWidth, hv_ImgHeight, hv_COLORS, hv_COLORS_NUMBER;
//	HTuple  hv_mButton, hv_ButtonPrev, hv_current, hv_finishFlag;
//	HTuple  hv_i, hv_Area, hv_Row, hv_Column, hv_po, hv_mousePositionRegionId;
//	HTuple  hv_userDefindRegionId, hv_isInside, hv_repeatFlag;
//	HTuple  hv_countNumber;
//
//	hv_WindowIDOut = hv_WindowID;
//	CountObj(ho_GenRegions, &hv_RegionNumber);
//	(*hv_regionIdDefinedByUser) = HTuple();
//	{
//		HTuple end_val3 = hv_RegionNumber - 1;
//		HTuple step_val3 = 1;
//		for (hv_index = 0; hv_index.Continue(end_val3, step_val3); hv_index += step_val3)
//		{
//			(*hv_regionIdDefinedByUser) = (*hv_regionIdDefinedByUser).TupleConcat(-1);
//		}
//	}
//	GetImageSize(ho_Image, &hv_ImgWidth, &hv_ImgHeight);
//	hv_WindowIDOut = hv_WindowIDOut;
//	hv_COLORS.Clear();
//	hv_COLORS[0] = "#FF3232";
//	hv_COLORS[1] = "#FF9F00";
//	hv_COLORS[2] = "#59E3FF";
//	hv_COLORS[3] = "#7D32FF";
//	hv_COLORS[4] = "#0C88FC";
//	hv_COLORS[5] = "#081DA5";
//	hv_COLORS[6] = "#CCA20A";
//	hv_COLORS[7] = "#0A24CC";
//	hv_COLORS[8] = "#FC0C0C";
//	hv_COLORS[9] = "#CC00FF";
//	TupleLength(hv_COLORS, &hv_COLORS_NUMBER);
//	hv_mButton = 0;
//	hv_ButtonPrev = 0;
//	(*hv_selectedRegionIdList) = HTuple();
//	if (HDevWindowStack::IsOpen())
//		ClearWindow(HDevWindowStack::GetActive());
//	hv_current = 0;
//	hv_finishFlag = 0;
//
//	while (0 != (0 != 1))
//	{
//		if (0 != hv_finishFlag)
//		{
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "black");
//			break;
//		}
//		hv_mousePositionRegionId = -1;
//		if (0 != (hv_current != 0))
//		{
//			hv_mousePositionRegionId = hv_current;
//		}
//		dev_update_on();
//		HDevWindowStack::SetActive(hv_WindowIDOut);
//		if (HDevWindowStack::IsOpen())
//			DispObj(ho_Image, HDevWindowStack::GetActive());
//
//		if (HDevWindowStack::IsOpen())
//			SetLineWidth(HDevWindowStack::GetActive(), 2);
//		{
//			HTuple end_val64 = hv_RegionNumber;
//			HTuple step_val64 = 1;
//			for (hv_i = 1; hv_i.Continue(end_val64, step_val64); hv_i += step_val64)
//			{
//				SelectObj(ho_GenContours, &ho_cnt, hv_i);
//				if (0 != (hv_i == hv_current))
//				{
//					if (HDevWindowStack::IsOpen())
//						SetLineWidth(HDevWindowStack::GetActive(), 4);
//					if (HDevWindowStack::IsOpen())
//						SetColor(HDevWindowStack::GetActive(), "red");
//					if (HDevWindowStack::IsOpen())
//						DispObj(ho_cnt, HDevWindowStack::GetActive());
//					AreaCenterXld(ho_cnt, &hv_Area, &hv_Row, &hv_Column, &hv_po);
//					SetTposition(hv_WindowIDOut, hv_Row, hv_Column);
//					hv_userDefindRegionId = ((const HTuple&)(*hv_regionIdDefinedByUser))[hv_i - 1];
//					if (0 != (hv_userDefindRegionId != -1))
//					{
//						WriteString(hv_WindowIDOut, "ROI-" + hv_userDefindRegionId);
//					}
//					else
//					{
//						WriteString(hv_WindowIDOut, "ROI-" + hv_i);
//					}
//					if (HDevWindowStack::IsOpen())
//						SetLineWidth(HDevWindowStack::GetActive(), 2);
//				}
//				else
//				{
//					if (HDevWindowStack::IsOpen())
//						SetLineWidth(HDevWindowStack::GetActive(), 3);
//					if (HDevWindowStack::IsOpen())
//						SetColor(HDevWindowStack::GetActive(), HTuple(hv_COLORS[hv_i%hv_COLORS_NUMBER]));
//					AreaCenterXld(ho_cnt, &hv_Area, &hv_Row, &hv_Column, &hv_po);
//					SetTposition(hv_WindowIDOut, hv_Row, hv_Column);
//					WriteString(hv_WindowIDOut, "ROI-" + hv_i);
//					if (HDevWindowStack::IsOpen())
//						DispObj(ho_cnt, HDevWindowStack::GetActive());
//				}
//			}
//		}
//		if (0 != ((*hv_selectedRegionIdList) != HTuple()))
//		{
//			SelectObj(ho_GenContours, &ho_selectedContours, (*hv_selectedRegionIdList));
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "green");
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 4);
//			if (HDevWindowStack::IsOpen())
//				DispObj(ho_selectedContours, HDevWindowStack::GetActive());
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 1);
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "black");
//		}
//		if (HDevWindowStack::IsOpen())
//			SetLineWidth(HDevWindowStack::GetActive(), 1);
//		if (HDevWindowStack::IsOpen())
//			SetColor(HDevWindowStack::GetActive(), "black");
//		dev_update_off();
//
//		//*********************************************************************
//		// Error variable 'hv_Error' activated
//		hv_Error = 2;
//
//		try
//		{
//			hv_Error = 2;
//			GetMbutton(hv_WindowID, &hv_Row, &hv_Column, &hv_mButton);
//		}
//		catch (HalconCpp::HException e)
//		{
//			hv_Error = (int)e.ErrorCode();
//			if (hv_Error < 0)
//				throw e;
//		}
//
//		// Error variable 'hv_Error' deactivated
//		// dev_set_check ("give_error")
//		if (0 != (hv_Error == 2))
//		{
//		}
//		//*** TODO：一次只添加一个
//		hv_current = 0;
//		{
//			HTuple end_val116 = hv_RegionNumber;
//			HTuple step_val116 = 1;
//			for (hv_i = 1; hv_i.Continue(end_val116, step_val116); hv_i += step_val116)
//			{
//				hv_isInside = 0;
//				SelectObj(ho_GenContours, &ho_contourSelected, hv_i);
//				TestXldPoint(ho_contourSelected, hv_Row, hv_Column, &hv_isInside);
//				if (0 != (hv_isInside == 1))
//				{
//					if (0 != (hv_i>hv_current))
//					{
//						hv_current = hv_i;
//					}
//				}
//			}
//		}
//		if (0 != (hv_mButton != 0))
//		{
//			if (0 != (HTuple(hv_mButton == 1).TupleAnd(hv_current>0)))
//			{
//				hv_repeatFlag = 0;
//				TupleLength((*hv_selectedRegionIdList), &hv_countNumber);
//				if (0 != (hv_countNumber == 0))
//				{
//					(*hv_selectedRegionIdList) = hv_current;
//				}
//				else
//				{
//					{
//						HTuple end_val133 = hv_countNumber - 1;
//						HTuple step_val133 = 1;
//						for (hv_index = 0; hv_index.Continue(end_val133, step_val133); hv_index += step_val133)
//						{
//							if (0 != (HTuple((*hv_selectedRegionIdList)[hv_index]) == hv_current))
//							{
//								hv_repeatFlag = 1;
//							}
//						}
//					}
//					if (0 != (hv_repeatFlag == 0))
//					{
//						(*hv_selectedRegionIdList) = (*hv_selectedRegionIdList).TupleConcat(hv_current);
//					}
//				}
//				if (0 != (hv_mButton == 4))
//				{
//					hv_finishFlag = 1;
//				}
//			}
//		}
//		//** dump_window_image(InitWindowImage,WindowIDOut)
//		dev_update_on();
//	}
//	if (HDevWindowStack::IsOpen())
//		CloseWindow(HDevWindowStack::Pop());
//	return;
//}

//void get_user_selected_ROIs(HObject ho_Image, HObject ho_GenRegions, HObject ho_GenContours,
//	HTuple hv_WindowID, HTuple *hv_regionIdDefinedByUser, HTuple *hv_selectedRegionIdList,
//	HTuple *hv_mousePositionRegionID)
//{
//
//	// Local iconic variables
//	HObject  ho_cnt, ho_selectedContours, ho_contourSelected;
//
//	// Local control variables
//	HTuple  hv_Error;
//	HTuple  hv_WindowIDOut, hv_RegionNumber, hv_index;
//	HTuple  hv_ImgWidth, hv_ImgHeight, hv_COLORS, hv_COLORS_NUMBER;
//	HTuple  hv_mButton, hv_ButtonPrev, hv_current, hv_finishFlag;
//	HTuple  hv_i, hv_Area, hv_Row, hv_Column, hv_po, hv_mousePositionRegionId;
//	HTuple  hv_userDefindRegionId, hv_isInside, hv_repeatFlag;
//	HTuple  hv_countNumber, hv_cntN;
//
//	hv_WindowIDOut = hv_WindowID;
//	CountObj(ho_GenRegions, &hv_RegionNumber);
//	(*hv_regionIdDefinedByUser) = HTuple();
//	{
//		HTuple end_val3 = hv_RegionNumber - 1;
//		HTuple step_val3 = 1;
//		for (hv_index = 0; hv_index.Continue(end_val3, step_val3); hv_index += step_val3)
//		{
//			(*hv_regionIdDefinedByUser) = (*hv_regionIdDefinedByUser).TupleConcat(-1);
//		}
//	}
//	GetImageSize(ho_Image, &hv_ImgWidth, &hv_ImgHeight);
//	hv_WindowIDOut = hv_WindowIDOut;
//	hv_COLORS.Clear();
//	hv_COLORS[0] = "#FF3232";
//	hv_COLORS[1] = "#FF9F00";
//	hv_COLORS[2] = "#59E3FF";
//	hv_COLORS[3] = "#7D32FF";
//	hv_COLORS[4] = "#0C88FC";
//	hv_COLORS[5] = "#081DA5";
//	hv_COLORS[6] = "#CCA20A";
//	hv_COLORS[7] = "#0A24CC";
//	hv_COLORS[8] = "#FC0C0C";
//	hv_COLORS[9] = "#CC00FF";
//	TupleLength(hv_COLORS, &hv_COLORS_NUMBER);
//	hv_mButton = 0;
//	hv_ButtonPrev = 0;
//	(*hv_selectedRegionIdList) = HTuple();
//	if (HDevWindowStack::IsOpen())
//		ClearWindow(HDevWindowStack::GetActive());
//	hv_current = 0;
//	hv_finishFlag = 0;
//	//dev_set_window (WindowIDOut)
//	//disp_image (Image, WindowIDOut)
//	//dev_display (Image)
//	//dev_set_line_width (2)
//	//for i := 1 to RegionNumber by 1
//	//select_obj (GenContours, cnt, i)
//	//dev_set_color (COLORS[i%COLORS_NUMBER])
//	//if (i == current and DebugMode)
//	//dev_set_color ('red')
//	//dev_display (cnt)
//	//area_center_xld (cnt, Area, Row, Column, po)
//	//set_tposition (WindowIDOut, Row, Column)
//	//write_string (WindowIDOut, 'ROI-'+i)
//	//endif
//	//disp_xld (cnt, WindowIDOut)
//	//dev_display (cnt)
//	//endfor
//	//dump_window_image (InitWindowImage, WindowIDOut)
//
//	while (0 != (0 != 1))
//	{
//		if (0 != hv_finishFlag)
//		{
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "black");
//			break;
//		}
//		hv_mousePositionRegionId = -1;
//		if (0 != (hv_current != 0))
//		{
//			hv_mousePositionRegionId = hv_current;
//		}
//		dev_update_on();
//		HDevWindowStack::SetActive(hv_WindowIDOut);
//		if (HDevWindowStack::IsOpen())
//			DispObj(ho_Image, HDevWindowStack::GetActive());
//		//dev_set_line_width (2)
//		//for i := 1 to RegionNumber by 1
//		//select_obj (GenContours, cnt, i)
//		//dev_set_color (COLORS[i%COLORS_NUMBER])
//		//if (i == current and DebugMode)
//		//dev_set_color ('red')
//		//dev_display (cnt)
//		//area_center_xld (cnt, Area, Row, Column, po)
//		//set_tposition (WindowIDOut, Row, Column)
//		//write_string (WindowIDOut, 'ROI-'+i)
//		//endif
//		//disp_xld (cnt, WindowIDOut)
//		//dev_display (cnt)
//		//endfor
//		//stop ()
//
//		if (HDevWindowStack::IsOpen())
//			SetLineWidth(HDevWindowStack::GetActive(), 2);
//		{
//			HTuple end_val64 = hv_RegionNumber;
//			HTuple step_val64 = 1;
//			for (hv_i = 1; hv_i.Continue(end_val64, step_val64); hv_i += step_val64)
//			{
//				SelectObj(ho_GenContours, &ho_cnt, hv_i);
//				if (0 != (hv_i == hv_current))
//				{
//					if (HDevWindowStack::IsOpen())
//						SetLineWidth(HDevWindowStack::GetActive(), 4);
//					if (HDevWindowStack::IsOpen())
//						SetColor(HDevWindowStack::GetActive(), "red");
//					if (HDevWindowStack::IsOpen())
//						DispObj(ho_cnt, HDevWindowStack::GetActive());
//					AreaCenterXld(ho_cnt, &hv_Area, &hv_Row, &hv_Column, &hv_po);
//					SetTposition(hv_WindowIDOut, hv_Row, hv_Column);
//					hv_userDefindRegionId = ((const HTuple&)(*hv_regionIdDefinedByUser))[hv_i - 1];
//					if (0 != (hv_userDefindRegionId != -1))
//					{
//						WriteString(hv_WindowIDOut, "ROI-" + hv_userDefindRegionId);
//					}
//					else
//					{
//						WriteString(hv_WindowIDOut, "ROI-" + hv_i);
//					}
//					if (HDevWindowStack::IsOpen())
//						SetLineWidth(HDevWindowStack::GetActive(), 2);
//				}
//				else
//				{
//					if (HDevWindowStack::IsOpen())
//						SetLineWidth(HDevWindowStack::GetActive(), 3);
//					if (HDevWindowStack::IsOpen())
//						SetColor(HDevWindowStack::GetActive(), HTuple(hv_COLORS[hv_i%hv_COLORS_NUMBER]));
//					AreaCenterXld(ho_cnt, &hv_Area, &hv_Row, &hv_Column, &hv_po);
//					SetTposition(hv_WindowIDOut, hv_Row, hv_Column);
//					WriteString(hv_WindowIDOut, "ROI-" + hv_i);
//					if (HDevWindowStack::IsOpen())
//						DispObj(ho_cnt, HDevWindowStack::GetActive());
//				}
//			}
//		}
//		if (0 != ((*hv_selectedRegionIdList) != HTuple()))
//		{
//			SelectObj(ho_GenContours, &ho_selectedContours, (*hv_selectedRegionIdList));
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "green");
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 4);
//			if (HDevWindowStack::IsOpen())
//				DispObj(ho_selectedContours, HDevWindowStack::GetActive());
//			if (HDevWindowStack::IsOpen())
//				SetLineWidth(HDevWindowStack::GetActive(), 1);
//			if (HDevWindowStack::IsOpen())
//				SetColor(HDevWindowStack::GetActive(), "black");
//		}
//		if (HDevWindowStack::IsOpen())
//			SetLineWidth(HDevWindowStack::GetActive(), 1);
//		if (HDevWindowStack::IsOpen())
//			SetColor(HDevWindowStack::GetActive(), "black");
//		dev_update_off();
//		//stop ()
//		//*********************************************************************
//		//*********************************************************************
//		// Error variable 'hv_Error' activated
//		hv_Error = 2;
//		// dev_set_check ("~give_error")
//		//*********************************************************************
//		//*********************************************************************
//		try
//		{
//			hv_Error = 2;
//			GetMbutton(hv_WindowID, &hv_Row, &hv_Column, &hv_mButton);
//		}
//		catch (HalconCpp::HException e)
//		{
//			hv_Error = (int)e.ErrorCode();
//			if (hv_Error < 0)
//				throw e;
//		}
//		//get_mposition (WindowIDOut, Row, Column, mButton)
//		//*********************************************************************
//		//*********************************************************************
//		// Error variable 'hv_Error' deactivated
//		// dev_set_check ("give_error")
//		if (0 != (hv_Error == 2))
//		{
//		}
//		//*** TODO：一次只添加一个
//		hv_current = 0;
//		{
//			HTuple end_val116 = hv_RegionNumber;
//			HTuple step_val116 = 1;
//			for (hv_i = 1; hv_i.Continue(end_val116, step_val116); hv_i += step_val116)
//			{
//				hv_isInside = 0;
//				SelectObj(ho_GenContours, &ho_contourSelected, hv_i);
//				TestXldPoint(ho_contourSelected, hv_Row, hv_Column, &hv_isInside);
//				if (0 != (hv_isInside == 1))
//				{
//					if (0 != (hv_i>hv_current))
//					{
//						hv_current = hv_i;
//					}
//				}
//			}
//		}
//		if (0 != (hv_mButton != 0))
//		{
//			if (0 != (HTuple(hv_mButton == 1).TupleAnd(hv_current>0)))
//			{
//				hv_repeatFlag = 0;
//				TupleLength((*hv_selectedRegionIdList), &hv_countNumber);
//				if (0 != (hv_countNumber == 0))
//				{
//					(*hv_selectedRegionIdList) = hv_current;
//				}
//				else
//				{
//					{
//						HTuple end_val133 = hv_countNumber - 1;
//						HTuple step_val133 = 1;
//						for (hv_index = 0; hv_index.Continue(end_val133, step_val133); hv_index += step_val133)
//						{
//							if (0 != (HTuple((*hv_selectedRegionIdList)[hv_index]) == hv_current))
//							{
//								hv_repeatFlag = 1;
//							}
//						}
//					}
//					if (0 != (hv_repeatFlag == 0))
//					{
//						(*hv_selectedRegionIdList) = (*hv_selectedRegionIdList).TupleConcat(hv_current);
//					}
//				}
//			}
//			if (0 != (hv_mButton == 4))
//			{
//				if (0 != (hv_current != 0))
//				{
//					TupleLength((*hv_selectedRegionIdList), &hv_cntN);
//					if (0 != (hv_cntN == 0))
//					{
//						(*hv_selectedRegionIdList) = HTuple();
//					}
//					else
//					{
//						{
//							HTuple end_val149 = hv_cntN;
//							HTuple step_val149 = 1;
//							for (hv_index = 1; hv_index.Continue(end_val149, step_val149); hv_index += step_val149)
//							{
//								if (0 != (HTuple((*hv_selectedRegionIdList)[hv_index - 1]) == hv_current))
//								{
//									TupleRemove((*hv_selectedRegionIdList), hv_index - 1, &(*hv_selectedRegionIdList));
//									break;
//								}
//							}
//						}
//					}
//				}
//				else
//				{
//					//finishFlag := true
//				}
//			}
//		}
//		//** dump_window_image(InitWindowImage,WindowIDOut)
//		dev_update_on();
//	}
//	//dev_close_window ()
//	return;
//}


void gen_regions_by_contours_bingary_image(HObject ho_ImageBG, HObject ho_Contours,
	HObject *ho_GenRegions, HObject *ho_GenContours, HTuple hv_Width, HTuple hv_Height,
	HTuple *hv_RegionNumber)
{

	// Local iconic variables
	HObject  ho_ContoursOut, ho_ContoursImageWindow;
	HObject  ho_ContoursImage, ho_Regions;

	// Local control variables
	HTuple  hv_SaveImageWindowID, hv_thv;

	ho_ContoursOut = ho_Contours;
	SetWindowAttr("background_color", "white");
	OpenWindow(0, 0, hv_Width, hv_Height, 0, "", "", &hv_SaveImageWindowID);
	HDevWindowStack::Push(hv_SaveImageWindowID);
	ZoomImageSize(ho_ImageBG, &ho_ImageBG, hv_Width, hv_Height, "constant");
	if (HDevWindowStack::IsOpen())
		DispObj(ho_ImageBG, HDevWindowStack::GetActive());

	UnionAdjacentContoursXld(ho_ContoursOut, &ho_ContoursOut, 2, 2, "attr_keep");
	if (HDevWindowStack::IsOpen())
		SetColor(HDevWindowStack::GetActive(), "black");
	if (HDevWindowStack::IsOpen())
		SetLineWidth(HDevWindowStack::GetActive(), 2);
	if (HDevWindowStack::IsOpen())
		DispObj(ho_ContoursOut, HDevWindowStack::GetActive());

	DumpWindowImage(&ho_ContoursImageWindow, hv_SaveImageWindowID);
	CopyImage(ho_ContoursImageWindow, &ho_ContoursImage);
	ZoomImageSize(ho_ContoursImage, &ho_ContoursImage, hv_Width, hv_Height, "constant");
	BinaryThreshold(ho_ContoursImage, &ho_Regions, "max_separability", "dark", &hv_thv);
	BackgroundSeg(ho_Regions, &(*ho_GenRegions));

	CountObj((*ho_GenRegions), &(*hv_RegionNumber));
	SortRegion((*ho_GenRegions), &(*ho_GenRegions), "upper_left", "true", "row");
	GenContourRegionXld((*ho_GenRegions), &(*ho_GenContours), "border");
	if (HDevWindowStack::IsOpen())
		CloseWindow(HDevWindowStack::Pop());
	return;
}



void get_user_selected_ROIs(HObject ho_Image, HObject ho_GenRegions, HObject ho_GenContours,
	HTuple hv_WindowID, HTuple *hv_regionIdDefinedByUser, HTuple *hv_selectedRegionIdList,
	HTuple *hv_mousePositionRegionID)
{

	// Local iconic variables
	HObject  ho_cnt, ho_selectedRegion, ho_contourSelected;

	// Local control variables
	HTuple  hv_Error;
	HTuple  hv_WindowIDOut, hv_RegionNumber, hv_index;
	HTuple  hv_ImgWidth, hv_ImgHeight, hv_COLORS, hv_COLORS_NUMBER;
	HTuple  hv_mButton, hv_ButtonPrev, hv_current, hv_finishFlag;
	HTuple  hv_mousePositionRegionId, hv_i, hv_Area, hv_Row;
	HTuple  hv_Column, hv_po, hv_userDefindRegionId, hv_objCount;
	HTuple  hv_id, hv_isInside, hv_repeatFlag, hv_countNumber;
	HTuple  hv_cntNumber;

	hv_WindowIDOut = hv_WindowID;
	CountObj(ho_GenRegions, &hv_RegionNumber);
	(*hv_regionIdDefinedByUser) = HTuple();
	{
		HTuple end_val3 = hv_RegionNumber - 1;
		HTuple step_val3 = 1;
		for (hv_index = 0; hv_index.Continue(end_val3, step_val3); hv_index += step_val3)
		{
			(*hv_regionIdDefinedByUser) = (*hv_regionIdDefinedByUser).TupleConcat(-1);
		}
	}
	GetImageSize(ho_Image, &hv_ImgWidth, &hv_ImgHeight);
	hv_WindowIDOut = hv_WindowIDOut;
	hv_COLORS.Clear();
	hv_COLORS[0] = "#FF3232";
	hv_COLORS[1] = "#FF9F00";
	hv_COLORS[2] = "#59E3FF";
	hv_COLORS[3] = "#7D32FF";
	hv_COLORS[4] = "#0C88FC";
	hv_COLORS[5] = "#081DA5";
	hv_COLORS[6] = "#CCA20A";
	hv_COLORS[7] = "#0A24CC";
	hv_COLORS[8] = "#FC0C0C";
	hv_COLORS[9] = "#CC00FF";
	TupleLength(hv_COLORS, &hv_COLORS_NUMBER);
	hv_mButton = 0;
	hv_ButtonPrev = 0;
	(*hv_selectedRegionIdList) = HTuple();
	if (HDevWindowStack::IsOpen())
		ClearWindow(HDevWindowStack::GetActive());
	hv_current = 0;
	hv_finishFlag = 0;

	while (0 != (0 != 1))
	{
		if (0 != hv_finishFlag)
		{
			if (HDevWindowStack::IsOpen())
				SetColor(HDevWindowStack::GetActive(), "black");
			break;
		}
		hv_mousePositionRegionId = -1;
		if (0 != (hv_current != 0))
		{
			hv_mousePositionRegionId = hv_current;
		}
		dev_update_on();
		HDevWindowStack::SetActive(hv_WindowIDOut);
		if (HDevWindowStack::IsOpen())
			DispObj(ho_Image, HDevWindowStack::GetActive());
		if (HDevWindowStack::IsOpen())
			SetLineWidth(HDevWindowStack::GetActive(), 2);
		{
			HTuple end_val30 = hv_RegionNumber;
			HTuple step_val30 = 1;
			for (hv_i = 1; hv_i.Continue(end_val30, step_val30); hv_i += step_val30)
			{
				SelectObj(ho_GenContours, &ho_cnt, hv_i);
				if (0 != (hv_i == hv_current))
				{
					if (HDevWindowStack::IsOpen())
						SetLineWidth(HDevWindowStack::GetActive(), 4);
					if (HDevWindowStack::IsOpen())
						SetColor(HDevWindowStack::GetActive(), "red");
					if (HDevWindowStack::IsOpen())
						DispObj(ho_cnt, HDevWindowStack::GetActive());
					AreaCenterXld(ho_cnt, &hv_Area, &hv_Row, &hv_Column, &hv_po);
					SetTposition(hv_WindowIDOut, hv_Row, hv_Column);
					hv_userDefindRegionId = ((const HTuple&)(*hv_regionIdDefinedByUser))[hv_i - 1];
					if (0 != (hv_userDefindRegionId != -1))
					{
						WriteString(hv_WindowIDOut, "ROI-" + hv_userDefindRegionId);
					}
					else
					{
						WriteString(hv_WindowIDOut, "ROI-" + hv_i);
					}
					if (HDevWindowStack::IsOpen())
						SetLineWidth(HDevWindowStack::GetActive(), 2);
				}
				else
				{
					if (HDevWindowStack::IsOpen())
						SetLineWidth(HDevWindowStack::GetActive(), 3);
					if (HDevWindowStack::IsOpen())
						SetColor(HDevWindowStack::GetActive(), HTuple(hv_COLORS[hv_i%hv_COLORS_NUMBER]));
					AreaCenterXld(ho_cnt, &hv_Area, &hv_Row, &hv_Column, &hv_po);
					SetTposition(hv_WindowIDOut, hv_Row, hv_Column);
					WriteString(hv_WindowIDOut, "ROI-" + hv_i);
					if (HDevWindowStack::IsOpen())
						DispObj(ho_cnt, HDevWindowStack::GetActive());
				}
			}
		}
		if (0 != ((*hv_selectedRegionIdList) != HTuple()))
		{
			if (HDevWindowStack::IsOpen())
				SetLineWidth(HDevWindowStack::GetActive(), 4);
			TupleLength((*hv_selectedRegionIdList), &hv_objCount);
			if (0 != (hv_objCount>0))
			{
				{
					HTuple end_val58 = hv_objCount;
					HTuple step_val58 = 1;
					for (hv_index = 1; hv_index.Continue(end_val58, step_val58); hv_index += step_val58)
					{
						hv_id = ((const HTuple&)(*hv_selectedRegionIdList))[hv_index - 1];
						if (HDevWindowStack::IsOpen())
							SetColor(HDevWindowStack::GetActive(), HTuple(hv_COLORS[hv_id%hv_COLORS_NUMBER]));
						SelectObj(ho_GenRegions, &ho_selectedRegion, hv_id);
						if (HDevWindowStack::IsOpen())
							DispObj(ho_selectedRegion, HDevWindowStack::GetActive());
					}
				}
			}
			if (HDevWindowStack::IsOpen())
				SetLineWidth(HDevWindowStack::GetActive(), 1);
			if (HDevWindowStack::IsOpen())
				SetColor(HDevWindowStack::GetActive(), "black");
		}
		if (HDevWindowStack::IsOpen())
			SetLineWidth(HDevWindowStack::GetActive(), 1);
		if (HDevWindowStack::IsOpen())
			SetColor(HDevWindowStack::GetActive(), "black");
		dev_update_off();
		//*********************************************************************
		//*********************************************************************
		// Error variable 'hv_Error' activated
		hv_Error = 2;
		// dev_set_check ("~give_error")
		//*********************************************************************
		//*********************************************************************
		try
		{
			hv_Error = 2;
			GetMbutton(hv_WindowID, &hv_Row, &hv_Column, &hv_mButton);
		}
		catch (HalconCpp::HException e)
		{
			hv_Error = (int)e.ErrorCode();
			if (hv_Error < 0)
				throw e;
		}
		//get_mposition (WindowIDOut, Row, Column, mButton)
		//*********************************************************************
		//*********************************************************************
		// Error variable 'hv_Error' deactivated
		// dev_set_check ("give_error")
		if (0 != (hv_Error == 2))
		{
		}
		//*** TODO：一次只添加一个
		hv_current = 0;
		{
			HTuple end_val87 = hv_RegionNumber;
			HTuple step_val87 = 1;
			for (hv_i = 1; hv_i.Continue(end_val87, step_val87); hv_i += step_val87)
			{
				hv_isInside = 0;
				SelectObj(ho_GenContours, &ho_contourSelected, hv_i);
				TestXldPoint(ho_contourSelected, hv_Row, hv_Column, &hv_isInside);
				if (0 != (hv_isInside == 1))
				{
					if (0 != (hv_i>hv_current))
					{
						hv_current = hv_i;
					}
				}
			}
		}
		if (0 != (hv_mButton != 0))
		{
			if (0 != (HTuple(hv_mButton == 1).TupleAnd(hv_current>0)))
			{
				hv_repeatFlag = 0;
				TupleLength((*hv_selectedRegionIdList), &hv_countNumber);
				if (0 != (hv_countNumber == 0))
				{
					(*hv_selectedRegionIdList) = hv_current;
				}
				else
				{
					{
						HTuple end_val104 = hv_countNumber - 1;
						HTuple step_val104 = 1;
						for (hv_index = 0; hv_index.Continue(end_val104, step_val104); hv_index += step_val104)
						{
							if (0 != (HTuple((*hv_selectedRegionIdList)[hv_index]) == hv_current))
							{
								hv_repeatFlag = 1;
							}
						}
					}
					if (0 != (hv_repeatFlag == 0))
					{
						(*hv_selectedRegionIdList) = (*hv_selectedRegionIdList).TupleConcat(hv_current);
					}
				}
			}
			if (0 != (hv_mButton == 4))
			{
				if (0 != (hv_current != 0))
				{
					TupleLength((*hv_selectedRegionIdList), &hv_cntNumber);
					if (0 != (hv_cntNumber == 0))
					{
						(*hv_selectedRegionIdList) = HTuple();
					}
					else
					{
						{
							HTuple end_val120 = hv_cntNumber;
							HTuple step_val120 = 1;
							for (hv_index = 1; hv_index.Continue(end_val120, step_val120); hv_index += step_val120)
							{
								if (0 != (HTuple((*hv_selectedRegionIdList)[hv_index - 1]) == hv_current))
								{
									TupleRemove((*hv_selectedRegionIdList), hv_index - 1, &(*hv_selectedRegionIdList));
									break;
								}
							}
						}
					}
				}
				else
				{
					//finishFlag := true
				}
			}
		}
		dev_update_on();
	}
	//dev_close_window ()
	return;
}



void MultiRegions::OnBnClickedButton2()
{
	// TODO:  在此添加控件通知处理程序代码
	HTuple hv_DxfStatus, hv_RegionNumber;
	HTuple hv_ImagePath = ".\\location\\white.png";
	ReadImage(&ho_ImageBG, hv_ImagePath);
	/*ReadContourXldDxf(&ho_Contours, "C:\\Users\\AI-009\\Desktop\\新建文件夹\\result.dxf", HTuple(),
		HTuple(), &hv_DxfStatus);*/
	gen_regions_by_contours_bingary_image(ho_ImageBG, ho_Contours, &ho_GenRegions, &ho_GenContours, m_hWidth,
		m_hHeight, &hv_RegionNumber);
	get_user_selected_ROIs(hoImage, ho_GenRegions, ho_GenContours, hvWindowID, &hv_regionIdDefinedByUser, &hv_contourGroupIdx, &hv_mousePositionRegionID);
}

void MultiRegions::OnBnClickedButton3()
{
	// TODO:  在此添加控件通知处理程序代码
	int i;
	HTuple Substrings, length, s = HTuple("");
	const char *pStr = NULL;
	//CString strCS;
	TCHAR szBuffer[MAX_PATH] = { 0 };
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = NULL;
	bi.pszDisplayName = szBuffer;
	bi.lpszTitle = _T("从下面选择文件或文件夹:");
	bi.ulFlags = BIF_BROWSEINCLUDEFILES;
	LPITEMIDLIST idl = SHBrowseForFolder(&bi);
	if (NULL == idl)
	{
		return;
	}
	SHGetPathFromIDList(idl, szBuffer);
	//cout << HTuple(szBuffer).S() << endl;
	//pStr = HTuple(szBuffer).S().Text();
	//strCS.Format("%s", pStr);
	//cout << strCS << endl;
	TupleSplit(HTuple(szBuffer).S(), "\\", &Substrings);
	TupleLength(Substrings, &length);
	/*for (int i = 0; i < length.I(); i++)
		cout << Substrings[i].S() << endl;*/
	Substrings[length.I() - 1] = HTuple("result");
	//cout << HTuple(szBuffer).S() << endl;
	for (i = 0; i < length.I()-1; i++)
	{
		s += Substrings[i];
		s += "\\";
	}
	//cout << s.S() << endl;
	WriteContourXldDxf(ho_ModelTrans, s + Substrings[i]);
}
