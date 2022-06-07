#include <iostream>
#include <stdlib.h>


using namespace std;

namespace DSX{
	void nameTH1(TH1* h, string title, string x, string y = "Counts"){

		h->GetXaxis()->SetTitle(x.c_str());
		h->GetYaxis()->SetTitle(y.c_str());
		h->SetTitle(title.c_str());
		return;
	}
}