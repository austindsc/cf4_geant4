

#include <iostream>
#include <stdlib.h>

#include "adsxTools.C"


using namespace std;

// namespace adsx{
// 	void nameTH1(TH1* h, string title, string x, string y = "Counts"){

// 		h->GetXaxis()->SetTitle(x.c_str());
// 		h->GetYaxis()->SetTitle(y.c_str());
// 		h->SetTitle(title.c_str());
// 		return;
// 	}
// }

int compAuFracs(){

	// 3V data
	TFile * f10 = TFile::Open("../compAuFrac/eDep_Au10_100k.root", "READ");
	TTree * t10 = (TTree*) f10->Get("Edep");

	TFile * f50 = TFile::Open("../compAuFrac/eDep_Au50_100k.root", "READ");
	TTree * t50 = (TTree*) f50->Get("Edep");

	TFile * f90 = TFile::Open("../compAuFrac/eDep_Au90_100k.root", "READ");
	TTree * t90 = (TTree*) f90->Get("Edep");


	int nbins = 250;
	float Elow = 0;
	float Ehigh = 5000; // in kev
	TH1 * h10_edep = new TH1F("h10_edep", "10\% gold", nbins, Elow, Ehigh);
	DSX::nameTH1(h10_edep, "yoo", "Energy (KeV)");

	t10->Draw("fEdepInCyl_kev>>h10_edep", "", "hist");


	cout << "yo dog" << endl;
	return 0;
}