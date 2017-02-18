#include <RichEdit/RichEdit.h>
#include <PdfDraw/PdfDraw.h>

using namespace Upp;

#define IMAGECLASS UWordImg
#define IMAGEFILE  <UWord/UWord.iml>
#include <Draw/iml.h>

FileSel& UWordFs()
{
	static FileSel fs;
	return fs;
}

FileSel& PdfFs()
{
	static FileSel fs;
	return fs;
}

class UWord : public TopWindow {
public:
	virtual void DragAndDrop(Point, PasteClip& d);
	virtual void FrameDragAndDrop(Point, PasteClip& d);

protected:
	RichEdit   editor;
	MenuBar    menubar;
	ToolBar    toolbar;
	StatusBar  statusbar;
	String     filename;

	static LRUList& lrufile() { static LRUList l; return l; }

	void Load(const String& filename);
	void OpenFile(const String& fn);
	void New();
	void Open();
	void Save0();
	void Save();
	void SaveAs();
	void Print();
	void Pdf();
	void About();
	void Destroy();
	void SetBar();
	void FileBar(Bar& bar);
	void AboutMenu(Bar& bar);
	void MainMenu(Bar& bar);
	void MainBar(Bar& bar);

public:
	typedef UWord CLASSNAME;

	static void SerializeApp(Stream& s);

	UWord();
};

void UWord::FileBar(Bar& bar)
{
	bar.Add("Новый", CtrlImg::new_doc(), THISBACK(New))
	   .Key(K_CTRL_N)
	   .Help("Открыть новое окно");
	bar.Add("Открыть..", CtrlImg::open(), THISBACK(Open))
	   .Key(K_CTRL_O)
	   .Help("Открыть существующий документ");
	bar.Add(editor.IsModified(), "Сохранить", CtrlImg::save(), THISBACK(Save))
	   .Key(K_CTRL_S)
	   .Help("Сохранить текущий документ");
	bar.Add("Сохранить Как", CtrlImg::save_as(), THISBACK(SaveAs))
	   .Help("Сохранить текущий документ под новым названием");
	bar.ToolGap();
	bar.MenuSeparator();
	bar.Add("Печать..", CtrlImg::print(), THISBACK(Print))
	   .Key(K_CTRL_P)
	   .Help("Напечатать документ");
	bar.Add("Экспортировать в PDF..", UWordImg::pdf(), THISBACK(Pdf))
	   .Help("Экспортировать документ в файл PDF");
	if(bar.IsMenuBar()) {
		if(lrufile().GetCount())
			lrufile()(bar, THISBACK(OpenFile));
		bar.Separator();
		bar.Add("Выход", THISBACK(Destroy));
	}
}

void UWord::AboutMenu(Bar& bar)
{
	bar.Add("О программе..", THISBACK(About));
}

void UWord::MainMenu(Bar& bar)
{
	bar.Add("Файл", THISBACK(FileBar));
	bar.Add("Окно", callback(WindowsMenu));
	bar.Add("Помощь", THISBACK(AboutMenu));
}

void UWord::New()
{
	new UWord;
}

bool IsRTF(const char *fn)
{
	return ToLower(GetFileExt(fn)) == ".rtf";
}

void UWord::Load(const String& name)
{
	lrufile().NewEntry(name);
	if(IsRTF(name))
		editor.Pick(ParseRTF(LoadFile(name)));
	else
		editor.SetQTF(LoadFile(name));
	filename = name;
	editor.ClearModify();
	Title(filename);
}

void UWord::OpenFile(const String& fn)
{
	if(filename.IsEmpty() && !editor.IsModified())
		Load(fn);
	else
		(new UWord)->Load(fn);
}

void UWord::Open()
{
	FileSel& fs = UWordFs();
	if(fs.ExecuteOpen())
		OpenFile(fs);
	else
		statusbar.Temporary("Загрузка прервана.");
}

void UWord::DragAndDrop(Point, PasteClip& d)
{
	if(AcceptFiles(d)) {
		Vector<String> fn = GetFiles(d);
		for(int i = 0; i < fn.GetCount(); i++)
			if(FileExists(fn[i]))
				OpenFile(fn[i]);
	}
}

void UWord::FrameDragAndDrop(Point p, PasteClip& d)
{
	DragAndDrop(p, d);
}

void UWord::Save0()
{
	lrufile().NewEntry(filename);
	if(filename.IsEmpty())
		SaveAs();
	else {
		if(SaveFile(filename, IsRTF(filename) ? EncodeRTF(editor.Get()) : editor.GetQTF())) {
			statusbar.Temporary("Файл " + filename + " сохранён.");
			ClearModify();
		}
		else
			Exclamation("Ошибка при сохранении файла [* " + DeQtf(filename) + "]!");
	}
}

void UWord::Save()
{
	if(!editor.IsModified()) return;
	Save0();
}

void UWord::SaveAs()
{
	FileSel& fs = UWordFs();
	if(fs.ExecuteSaveAs()) {
		filename = fs;
		Title(filename);
		Save0();
	}
}

void UWord::Print()
{
	editor.Print();
}

void UWord::Pdf()
{
	FileSel& fs = PdfFs();
	if(!fs.ExecuteSaveAs("Выходной файл PDF"))
		return;
	Size page = Size(3968, 6074);
	PdfDraw pdf;
	UPP::Print(pdf, editor.Get(), page);
	SaveFile(~fs, pdf.Finish());
}

void UWord::About()
{
	PromptOK("[A5 uWord]&Используется технология [*^www://upp.sf.net^ Ultimate`+`+].");
}

void UWord::Destroy()
{
	if(editor.IsModified()) {
		switch(PromptYesNoCancel("Сохранить изменения документа?")) {
		case 1:
			Save();
			break;
		case -1:
			return;
		}
	}
	delete this;
}

void UWord::MainBar(Bar& bar)
{
	FileBar(bar);
	bar.Separator();
	editor.DefaultBar(bar);
}

void UWord::SetBar()
{
	toolbar.Set(THISBACK(MainBar));
}

UWord::UWord()
{
	AddFrame(menubar);
	AddFrame(TopSeparatorFrame());
	AddFrame(toolbar);
	AddFrame(statusbar);
	Add(editor.SizePos());
	menubar.Set(THISBACK(MainMenu));
	Sizeable().Zoomable();
	WhenClose = THISBACK(Destroy);
	menubar.WhenHelp = toolbar.WhenHelp = statusbar;
	static int doc;
	Title(Format("Документ%d", ++doc));
	Icon(CtrlImg::File());
	editor.ClearModify();
	SetBar();
	editor.WhenRefreshBar = THISBACK(SetBar);
	OpenMain();
	ActiveFocus(editor);
}

void UWord::SerializeApp(Stream& s)
{
	int version = 1;
	s / version;
	s % UWordFs()
	  % PdfFs();
	if(version >= 1)
		s % lrufile();
}

GUI_APP_MAIN
{
	SetLanguage(LNG_ENGLISH);
	SetDefaultCharset(CHARSET_UTF8);

	UWordFs().Type("Файлы QTF", "*.qtf")
	         .Type("Файлы RTF", "*.rtf")
	         .AllFilesType()
	         .DefaultExt("qtf");
	PdfFs().Type("Файлы PDF", "*.pdf")
	       .AllFilesType()
	       .DefaultExt("pdf");

	LoadFromFile(callback(UWord::SerializeApp));
	new UWord;
	Ctrl::EventLoop();
	StoreToFile(callback(UWord::SerializeApp));
}
