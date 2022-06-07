

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
	TH1 * h50_edep = (TH1*) h10_edep->Clone("h50_edep");
	TH1 * h90_edep = (TH1*) h10_edep->Clone("h90_edep");

	DSX::nameTH1(h10_edep, "Energy Deposited in CF4", "Energy (KeV)");

	TCanvas * c1 = new TCanvas();
	t10->Draw("fEdepInCyl_kev>>h10_edep", "", "hist");
	t50->Draw("fEdepInCyl_kev>>h50_edep", "", "hist same");
	t90->Draw("fEdepInCyl_kev>>h90_edep", "", "hist same");

	h10_edep->SetLineColor(1);
	h50_edep->SetLineColor(2);
	h90_edep->SetLineColor(3);

	h10_edep->GetYaxis()->SetRangeUser(0, 2000);

	TLegend * leg1 = new TLegend();

	leg1->SetHeader("#splitline{Comparing Gold fraction}{in active matrix}", "C");
	leg1->AddEntry(h10_edep, "10\% Gold", "L");
	leg1->AddEntry(h50_edep, "50\% Gold", "L");
	leg1->AddEntry(h90_edep, "90\% Gold", "L");
	leg1->Draw("same");

	c1->Update();



	cout << "yo dog" << endl;
	return 0;
}