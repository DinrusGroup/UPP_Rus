#include "Days.h"

void Days::Compute()
{
	result = IsNull(date1) || IsNull(date2) ? "" :
	         Format("Всего %d дней между %` и %`", abs(Date(~date1) - Date(~date2)), ~date1, ~date2);
}

Days::Days()
{
	CtrlLayout(*this, "Дни");
	date1 <<= THISBACK(Compute);
	date2 <<= THISBACK(Compute);
	Compute();
}

GUI_APP_MAIN
{
	Days().Run();
}
