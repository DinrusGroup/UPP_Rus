#include "ide.h"

void Ide::ConsoleShow()
{
	ShowConsole();
	console.Sync();
}

void Ide::ConsoleSync()
{
	console.Sync();
}

Vector<String> Ide::PickErrors()
{
	return console.PickErrors();
}

void Ide::DoProcessEvents()
{
	ProcessEvents();
}

void Ide::ReQualifyCodeBase()
{
	::ReQualifyCodeBase();
}

String Ide::GetMain()
{
	return main;
}

void Ide::BeginBuilding(bool sync_files, bool clear_console)
{
	SetupDefaultMethod();
	HdependTimeDirty();
	Renumber();
	ShowConsole();
	StopDebug();
	SaveFile();
	SaveWorkspace();
	SetIdeState(BUILDING);
	console.Kill();
	console.ClearError();
	if(clear_console)
		console.Clear();
	build_time = GetTickCount();
	CreateHost(sync_files);
	cmdout.Clear();
}

void Ide::EndBuilding(bool ok)
{
	console.EndGroup();
	console.Wait();
	Vector<String> errors = console.PickErrors();
	CreateHost(false)->DeleteFile(errors);
	if(!errors.IsEmpty())
		ok = false;
	PutConsole("");
	PutConsole((ok ? "OK. " : "Имеются ошибки. ") + GetPrintTime(build_time));
	SetIdeState(EDITING);
	if(GetTopWindow()->IsOpen())
		if(ok)
			BeepMuteInformation();
		else
			BeepMuteExclamation();
	ShowConsole();
}

void Ide::DoBuild()
{
	Build();
}

void Ide::PackageBuild()
{
	BeginBuilding(true, true);
	const Workspace& wspc = IdeWorkspace();
	int pi = GetPackageIndex();
	if(pi >= 0 && pi <= wspc.GetCount()) {
		Vector<String> linkfile;
		String linkopt;
		bool ok = BuildPackage(wspc, pi, 0, 1, mainconfigparam, Null, linkfile, linkopt);
		EndBuilding(ok);
	}
}

void Ide::StopBuild()
{
	if(idestate == BUILDING) {
		console.Kill();
		PutConsole("Прервано пользователем.");
		SetIdeState(EDITING);
	}
}

String Ide::GetOutputDir()
{
	return GetFileFolder(target);
}

void Ide::PackageClean()
{
	const Workspace& wspc = IdeWorkspace();
	int pi = GetPackageIndex();
	if(pi >= 0 && pi < wspc.GetCount()) {
		console.Clear();
		CleanPackage(wspc, pi);
	}
}

void Ide::CleanUppOut()
{
	String out = GetVar("OUTPUT");
	if(!PromptYesNo(NFormat("Стереть всю папку вывода [* \1%s\1]?", out)))
		return;
	console.Clear();
	PutConsole("UPPOUT очистка...");
	DeleteFolderDeep(out);
	PutConsole("(готово)");
	HideBottom();
}

void Ide::FileCompile()
{
	if(editfile.IsEmpty())
		return;
	ClearErrorEditor(editfile);
	SwitchHeader();
	BeginBuilding(true, true);
	const Workspace& wspc = IdeWorkspace();
	bool ok = true;
	onefile = editfile;
	if(wspc.GetCount()) {
		Vector<String> linkfile;
		String linkopt;
		for(int i = 0; i < wspc.GetCount(); i++)
			BuildPackage(wspc, i, 1, wspc.GetCount(), mainconfigparam, Null, linkfile, linkopt, false);
	}
	onefile.Clear();
	EndBuilding(ok);
	SetErrorEditor();
}

void Ide::Preprocess(bool asmout) {
	if(editfile.IsEmpty())
		return;
	int pi = GetPackageIndex();
	if(pi < 0) return;
	SwitchHeader();
	String pfn = ConfigFile(GetFileTitle(editfile) + ".i.tmp");
	DeleteFile(pfn);
	const Workspace& wspc = IdeWorkspace();
	if(pi >= wspc.GetCount())
		return;
	One<Host> host = CreateHost(true);
	One<Builder> b = CreateBuilder(~host);
	Vector<String> linkfile;
	String linkopt;
	b->config = PackageConfig(wspc, pi, GetMethodVars(method), mainconfigparam, *host, *b);
	console.Clear();
	PutConsole((asmout ? "Компилируется " : "Препроцессируется ") + editfile);
	b->Preprocess(wspc[pi], editfile, pfn, asmout);
	HideBottom();
	if(FileExists(pfn)) {
		EditFile(pfn);
		if(!editor.IsReadOnly())
			ToggleReadOnly();
	}
}

void Ide::CreateMakefile()
{
	const Workspace& wspc = IdeWorkspace();
	if(wspc.GetCount() == 0) {
		PutConsole("Проект пуст!");
		return;
	}
	FileSel mfout;
	mfout.AllFilesType();
	mfout <<= AppendFileName(GetFileDirectory(PackagePath(wspc[0])), "Makefile");
	if(!mfout.ExecuteSaveAs("Сохранить makefile как"))
		return;
	SaveMakeFile(~mfout, true);
}

void Ide::OpenOutputFolder()
{
	ShellOpenFolder(GetFileFolder(target));
}