#include "Common.h"

bool Debugger::Tip(const String&, CodeEditor::MouseTip&)
{
	return false;
}

bool FinishSave(String tmpfile, String outfile)
{
	Progress progress;
	int time = GetTickCount();
	for(;;) {
		progress.SetTotal(10000);
		progress.SetText("Сохраняется '" + GetFileName(outfile) + "'");
		if(!FileExists(tmpfile))
			return false;
		FileDelete(outfile);
		if(FileMove(tmpfile, outfile))
			return true;
		IdeConsoleFlush();
		Sleep(200);
		if(progress.SetPosCanceled((GetTickCount() - time) % progress.GetTotal())) {
			int art = Prompt(Ctrl::GetAppName(), CtrlImg::exclamation(),
				"Не удаётся сохранить текущий файл.&"
				"Повторить сохранение, игнорировать его или сохранить файл в другом месте?",
				"Сохранить как...", "Повторить", "Игнорировать");
			if(art < 0)
				return false;
			if(art && AnySourceFs().ExecuteSaveAs())
				outfile = AnySourceFs();
			progress.SetPos(0);
		}
	}
}

bool FinishSave(String outfile)
{
	return FinishSave(outfile + ".$tmp", outfile);
}

bool SaveFileFinish(const String& filename, const String& data)
{
	if(!SaveFile(filename + ".$tmp", data)) {
		Exclamation("Ошибка при создании временного файла " + filename);
		return false;
	}
	return FinishSave(filename);
}

bool SaveChangedFileFinish(const String& filename, const String& data)
{
	if(data == LoadFile(filename))
		return true;
	return SaveFileFinish(filename, data);
}

typedef VectorMap<String, String> StringMap;

GLOBAL_VAR(StringMap,  sWorkspaceCfg)
GLOBAL_VAR(Vector<Callback>, sWorkspaceCfgFlush)

void    RegisterWorkspaceConfig(const char *name)
{
	ASSERT(sWorkspaceCfg().Find(name) < 0);
	sWorkspaceCfg().Add(name);
}

void    RegisterWorkspaceConfig(const char *name, Callback WhenFlush)
{
	RegisterWorkspaceConfig(name);
	sWorkspaceCfgFlush().Add(WhenFlush);
}


String& WorkspaceConfigData(const char *name)
{
	return sWorkspaceCfg().GetAdd(name);
}

void  SerializeWorkspaceConfigs(Stream& s)
{
	int i;
	for(i = 0; i < sWorkspaceCfgFlush().GetCount(); i++)
		sWorkspaceCfgFlush()[i]();
	VectorMap<String, String>& cfg = sWorkspaceCfg();
	int version = 0;
	s / version;
	int count = cfg.GetCount();
	s / count;
	for(i = 0; i < count; i++) {
		String name;
		if(s.IsStoring())
			name = cfg.GetKey(i);
		s % name;
		int q = cfg.Find(name);
		if(q >= 0)
			s % cfg[q];
		else
		{
			String dummy;
			s % dummy;
		}
	}
	s.Magic();
}

bool GuiPackageResolver(const String& error, const String& path, int line)
{
prompt:
	switch(Prompt(Ctrl::GetAppName(), CtrlImg::exclamation(),
	              error + "&при разборе пакета " + DeQtf(path),
		          "Редактировать \\& Повторить", "Игнорировать",  "Стоп")) {
	case 0:
		if(!PromptYesNo("Игнорирование повреждает пакет. Всё,находящеся после "
			            "точки ошибки будет потеряно.&Хотите продолжить ?"))
			goto prompt;
		return false;
	case 1: {
			TopWindow win;
			LineEdit edit;
			edit.Set(LoadFile(path));
			edit.SetCursor(edit.GetPos(line));
			win.Title(path);
			win.Add(edit.SizePos());
			win.Run();
			SaveFile(path, edit.Get());
		}
		return true;;
	case -1:
		exit(1);
	}
	return false;
}

void CleanModules()
{
	for(int i = 0; i < GetIdeModuleCount(); i++)
		GetIdeModule(i).CleanUsc();
}

bool IdeModuleUsc(CParser& p)
{
	for(int i = 0; i < GetIdeModuleCount(); i++)
		if(GetIdeModule(i).ParseUsc(p))
			return true;
	return false;
}

static void ReadMacro(CParser& p)
{
	IdeMacro macro;
	if(p.IsString()) {
		macro.menu = p.ReadString();
		if(p.Char(':'))
			macro.submenu = p.ReadString();
	}
	if(!p.IsChar('{'))
		macro.hotkey = ParseKeyDesc(p);
	EscLambda& l = macro.code.CreateLambda();
	const char *t = p.GetPtr();
	l.filename = p.GetFileName();
	l.line = p.GetLine();
	if(!p.Char('{'))
		p.ThrowError("отсутствует '{'");
	SkipBlock(p);
	l.code = String(t, p.GetPtr());
	Array<IdeMacro>& mlist = UscMacros();
	if(macro.hotkey) {
		int f = FindFieldIndex(mlist, &IdeMacro::hotkey, macro.hotkey);
		if(f >= 0) {
			PutConsole(NFormat("%s(%d): дублирующая горячая клавиша макроса %s\n", l.filename, l.line, GetKeyDesc(macro.hotkey)));
			const EscLambda& lambda = UscMacros()[f].code.GetLambda();
			PutConsole(NFormat("%s(%d): ранее определена здесь\n", lambda.filename, lambda.line));
		}
	}
	if(!IsNull(macro.menu)) {
		for(int i = 0; i < mlist.GetCount(); i++)
			if(mlist[i].menu == macro.menu && mlist[i].submenu == macro.submenu) {
				PutConsole(NFormat("%s(%d): дублирующий элемент меню макроса (%s:%s)\n",
					l.filename, l.line, macro.menu, macro.submenu));
				const EscLambda& lambda = UscMacros()[i].code.GetLambda();
				PutConsole(NFormat("%s(%d): ранее определён здесь\n", lambda.filename, lambda.line));
				break;
			}
	}
	mlist.Add(macro);
}

INITBLOCK {
	Package::SetPackageResolver(GuiPackageResolver);
	UscSetCleanModules(CleanModules);
	SetIdeModuleUsc(IdeModuleUsc);
	UscSetReadMacro(ReadMacro);
}

