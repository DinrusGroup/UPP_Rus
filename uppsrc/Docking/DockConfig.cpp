#include "Docking.h"

NAMESPACE_UPP

#define HIGHLIGHT_DURATION 1000

DockConfigDlg::DockConfigDlg(DockWindow& dockwindow) 
: dock(dockwindow),
  dockers(dockwindow.GetDockableCtrls()),
  menu(&dockwindow),
  highlight(NULL)
{
	CtrlLayoutOKCancel(*this, t_("Dockable Window Manager"));
	Sizeable().Zoomable();
	
	// Make backup of layout
	StringStream s;
	dock.SerializeLayout(s, true);
	backup = s;
	
	// Setup groups
	RefreshTree(true);
		
	// Setup layout list
	const ArrayMap<String, String>& l = dock.GetLayouts();
	for (int i = 0; i < l.GetCount(); i++)
		list.Add(l.GetKey(i));
	
	tree.MultiSelect(true);
	tree.WhenCursor 	= THISBACK(OnTreeCursor);
	tree.WhenBar 		= THISBACK(OnTreeContext);
	tree.WhenDropInsert = THISBACK(OnTreeDrop);
	tree.WhenDrag 		= THISBACK(OnTreeDrag);	
	
	list.MultiSelect(false);
	list.WhenSel	= THISBACK(OnListCursor);
	list.WhenSel	= THISBACK(OnListCursor);
	
	savelayout	<<= THISBACK(OnSaveLayout);
	loadlayout	<<= THISBACK(OnLoadLayout);
	deletelayout <<= THISBACK(OnDeleteLayout);
	newgroup 	<<= THISBACK(OnNewGroup);
	deletegroup <<= THISBACK(OnDeleteGroup);
	ok 			<<= THISBACK(OnOK);
	cancel		<<= THISBACK(OnCancel);	
	WhenClose 	=   THISBACK(OnCancel);
	locked		<<= THISBACK(OnLock);
	
	animatehighlight	<<= dock.IsAnimatedHighlight();
	animateframes 		<<= dock.IsAnimatedFrames();
	animatewindows 		<<= dock.IsAnimatedWindows();
	locked		<<= dock.IsLocked();
	tabbing 	<<= dock.IsTabbing();
	tabnesting 	<<= dock.IsNestedTabs();
	autohide 	<<= dock.IsAutoHide();
	grouping	<<= dock.IsGrouping();	
	
	closebtn	<<= dock.HasCloseButtons();
	autohidebtn	<<= dock.HasHideButtons();
	menubtn		<<= dock.HasMenuButtons();
	
	OnTreeCursor();
	OnListCursor();
}

void DockConfigDlg::RefreshTree(bool dogroups)
{
	TreeCtrl::Node n;
	
	if (dogroups) {
		tree.NoRoot(true).Clear();
		groups.Clear();
		n.Set(-1, t_("Все")).CanSelect(false).CanOpen(true);
		all = tree.Add(0, n);
		for (int i = 0; i < dockers.GetCount(); i++) {
			String s = dockers[i]->GetGroup();
			int group = s.IsEmpty() ? -1 : groups.Find(s);
			if (!s.IsEmpty() && group < 0) {
				n.Set(groups.GetCount(), s);
				group = tree.Add(0, n);
				tree.Open(group);
				groups.Add(s, group);
			}
		}	
		if (!groups.GetCount()) tree.Open(all);
	}
	else {
		for (int i = 0; i < tree.GetChildCount(0); i++)
			tree.RemoveChildren(tree.GetChild(0, i));
	}
	n.CanSelect(true).CanOpen(false);
	for (int i = 0; i < dockers.GetCount(); i++) {
		String s = dockers[i]->GetGroup();
		int group = s.IsEmpty() ? -1 : groups.Find(s);
		s = DockerString(dockers[i]);
		n.SetImage(dockers[i]->GetIcon()).Set(i, s);
		if (group >= 0) 
			tree.Add(groups[group], n);
		tree.Add(all, n);
	}
}

void DockConfigDlg::OnTreeContext(Bar& bar)
{
	int id = tree.GetCursor();
	if (id >= 0) {
		int p = tree.GetParent(id);
		if (p != 0) {
			menu.WindowMenu(bar, dockers[(int)tree.Get(id)]);
			bar.Separator();
		}
		menu.GroupMenu(bar, (String)tree.GetValue((p==0) ? id : p));
		if (p == 0 && id != all) {
			bar.Separator();
			bar.Add(t_("Удалить группу"), THISBACK1(DeleteGroup, id));	
		}			
	}
}

void DockConfigDlg::OnSaveLayout()
{
	int ix = ListIndex();
	String s = (ix >= 0) ? (String)list.Get(ix) : Null;
	if (EditText(s, t_("Новая Раскладка"), t_("Название раскладки:"), 25)) {
		if (!s.IsEmpty()) {
			ix = dock.GetLayouts().Find(s);
			if (ix < 0 || PromptOKCancel(Format(t_("Переписать раскладку '%s'?"), s))) {
				dock.SaveLayout(s);
				if (ix < 0) {
					list.Add(s);
					list.SetCursor(list.GetCount()-1);
					OnListCursor();
				}
			}
		}
	}
}

void DockConfigDlg::OnLoadLayout()
{
	Ctrl *focus = GetFocusCtrl();
	int ix = ListIndex();
	ASSERT(ix >= 0);
	dock.LoadLayout((String)list.Get(ix));
	dock.DisableFloating();
	if (focus) focus->SetFocus();
	RefreshTree(true);
}

void DockConfigDlg::OnDeleteLayout()
{
	int ix = ListIndex();
	if (!PromptOKCancel(Format(t_("Удалить раскладку '%s'?"), (String)list.Get(ix)))) return;
	dock.DeleteLayout((String)list.Get(ix));
	list.Remove(ix);
}

void DockConfigDlg::OnTreeCursor()
{
	int id = tree.GetCursor();
	if (id <= 0) { deletegroup.Disable(); return; }
	int p = tree.GetParent(id);
	if (p == 0)
		deletegroup.Enable();
	else
		Highlight(dockers[(int)tree.Get(id)]);
}


void DockConfigDlg::OnListCursor()
{
	loadlayout.Enable(list.GetCursor() >= 0);
	deletelayout.Enable(list.GetCursor() >= 0);
}

void DockConfigDlg::OnNewGroup()
{
	String s;
	if (EditText(s, t_("Новая Группа"), t_("Название группы:"), 25)) {
		if (!s.IsEmpty()) {
			if (groups.Find(s) < 0) {
				int id = tree.Add(0, Image(), Value(-1), Value(s));
				groups.Add(s, id);
				tree.SetCursor(list.GetCount()-1);
				OnTreeCursor();			
			}
			else {
				PromptOK(t_("Группа '%s' уже существует."));
				OnNewGroup();
			}
		}
	}
}

void DockConfigDlg::OnDeleteGroup()
{
	int id = tree.GetCursor();
	if (id < 0) return;
	int p = tree.GetParent(id);
	if (p != 0) id = p;
	if (id == all) return;
	DeleteGroup(id);
}

void DockConfigDlg::DeleteGroup(int id)
{	
	String s = (String)tree.GetValue(id);
	if (!PromptOKCancel(Format(t_("Удалить группу '%s'?"), s))) return;
	int ix = groups.Find(s);
	if (ix >= 0) {
		String g = Null;
		String name = groups.GetKey(ix);
		for (int i = 0; i < dockers.GetCount(); i++)
			if (dockers[i]->GetGroup() == name)
				dockers[i]->SetGroup(g);
		tree.RemoveChildren(id);
		tree.Remove(id);
	}
}

void DockConfigDlg::OnOK()
{
	dock.Animate(~animatehighlight, ~animateframes, ~animatewindows);
	dock.Tabbing(~tabbing);
	dock.NestedTabs(~tabnesting);
	dock.AutoHide(~autohide);
	dock.Grouping(~grouping);
	dock.WindowButtons(~menubtn, ~autohidebtn, ~closebtn);
	Close();		
	dock.EnableFloating();	
}

void DockConfigDlg::OnCancel()
{
	// Restore layout
	StringStream s(backup);
	dock.SerializeLayout(s, true);	
	Close();
}

void DockConfigDlg::OnLock()
{
	dock.LockLayout(locked);
}

void DockConfigDlg::OnTreeDrag()
{
	if (tree.GetCursor() >= 0 && tree.GetParent(tree.GetCursor()) == 0)
		return;
	tree.DoDragAndDrop(InternalClip(tree, "dockwindowdrag"),
	                       tree.GetDragSample(), DND_MOVE) == DND_MOVE;
}

void DockConfigDlg::OnTreeDrop(int parent, int ii, PasteClip& d)
{
	if (parent == 0)
		return d.Reject();
	int p = tree.GetParent(parent);
	if (p != 0)
		parent = p;
	
	if(AcceptInternal<TreeCtrl>(d, "dockwindowdrag")) {
		Vector<int> sel = tree.GetSel();
		for (int i = 0; i < sel.GetCount(); i++) {
			int ix = tree.Get(sel[i]);
			ASSERT(ix >= 0);
			dockers[ix]->SetGroup((parent == all) ? Null : (String)tree.GetValue(parent));
		}
		if (tree.GetCursor()) {
			int ix = tree.Get(tree.GetCursor());
			ASSERT(ix >= 0);
			dockers[ix]->SetGroup((parent == all) ? Null : (String)tree.GetValue(parent));			
		}
		tree.SetFocus();
		RefreshTree();
		return;
	}
}

void DockConfigDlg::Highlight(DockableCtrl *dc)
{
	if (dc != highlight) {
		if (DockCont *cont = dynamic_cast<DockCont *>(dc->GetParent())) {
			if (!highlight || cont != highlight->GetParent()) {
				StopHighlight();
				cont->Highlight();
			}
		}
		else {
			StopHighlight();
			dc->Highlight();
		}
		highlight = dc;
	}
	KillSetTimeCallback(HIGHLIGHT_DURATION, THISBACK(StopHighlight), 0);	
}

void DockConfigDlg::StopHighlight()
{
	if (highlight) {
		if (DockCont *dc = dynamic_cast<DockCont *>(highlight->GetParent()))
			dc->Refresh();
		else
			highlight->Refresh();
	}
	highlight = NULL;
}

int DockConfigDlg::ListIndex() const 
{
	if (!list.IsSelection()) {
		if (list.IsCursor())
			return list.GetCursor();
		else
			return -1;
	}
	for (int i = 0; i < list.GetCount(); i++)
		if (list.IsSelected(i))
			return i;
	return -1;
}


String DockConfigDlg::DockerString(DockableCtrl *dc) const
{
	return Format("%20<s %s", dc->GetTitle(), PositionString(dc));
}

String DockConfigDlg::PositionString(DockableCtrl *dc) const
{
	if (dc->IsFloating())
		return t_("Плавающее");
	else if (dc->IsTabbed())
		return t_("Вкладкой");	
	else if (dc->IsDocked()) {
		int align = dc->GetDockAlign();
		switch (align) {
		case DockWindow::DOCK_LEFT:
			return Format(t_("Помещено(%s)"), t_("Слева"));
		case DockWindow::DOCK_TOP:
			return Format(t_("Помещено(%s)"), t_("Вверху"));
		case DockWindow::DOCK_RIGHT:
			return Format(t_("Помещено(%s)"), t_("Справа"));
		case DockWindow::DOCK_BOTTOM:
			return Format(t_("Помещено(%s)"), t_("Внизу"));
		default:
			return t_("Помещено");
		}
	}
	else if (dc->IsAutoHide())
		return t_("Автоскрытие");
	else 
		return t_("Скрыто");
}

END_UPP_NAMESPACE
