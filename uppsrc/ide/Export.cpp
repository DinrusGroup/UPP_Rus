#include "ide.h"

void Ide::ExportMakefile(const String& ep)
{
	SaveMakeFile(AppendFileName(ep, "Makefile"), true);
}

void Ide::ExportProject(const String& ep, bool all, bool gui)
{
	SaveFile(false);
	::Workspace wspc;
	wspc.Scan(main);
	Index<String> used;
	HdependClearDependencies();
	for(int i = 0; i < wspc.GetCount(); i++) {
		const Package& p = wspc.GetPackage(i);
		String pn = wspc[i];
		for(int j = 0; j < p.GetCount(); j++) {
			const Package::File& f = p[j];
			if(!f.separator) {
				String p = SourcePath(pn, f);
				used.FindAdd(p);
				Vector<String> d = HdependGetDependencies(p);
				for(int q = 0; q < d.GetCount(); q++)
					used.FindAdd(d[q]);
				for(int q = 0; q < f.depends.GetCount(); q++)
					used.FindAdd(SourcePath(pn, f.depends[q].text));
			}
		}
		used.FindAdd(SourcePath(pn, "init"));
	}
	if(FileExists(ep)) {
		if(gui && !PromptYesNo(DeQtf(ep) + " существующий файл.&"
		                "Хотите его удалить?")) return;
		FileDelete(ep);
	}
	if(DirectoryExists(ep)) {
		if(gui && !PromptYesNo(DeQtf(ep) + " существующая папка.&"
		                "Хотите её заменить?")) return;
		DeleteFolderDeep(ep);
	}

	Progress pi("Проект экспортируется");
	pi.SetTotal(wspc.GetCount());
	for(int i = 0; i < wspc.GetCount(); i++) {
		if(gui && pi.StepCanceled())
			return;
		CopyFolder(AppendFileName(ep, wspc[i]), PackageDirectory(wspc[i]), used, all);
	}
	Vector<String> upp = GetUppDirs();
	for(int i = 0; i < upp.GetCount(); i++) {
		if(gui && pi.StepCanceled())
			return;
		String d = upp[i];
		FindFile ff(AppendFileName(d, "*"));
		while(ff) {
			if(ff.IsFile()) {
				String fn = ff.GetName();
				String path = AppendFileName(d, fn);
				if(all || used.Find(path) >= 0)
					Upp::SaveFile(AppendFileName(ep, fn), LoadFile(path));
			}
			ff.Next();
		}
		CopyFolder(AppendFileName(ep, wspc[i]), PackageDirectory(wspc[i]), used, all);
	}
	ExportMakefile(ep);
}