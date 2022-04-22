

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

int gammaComp_mar24(){

	TFile * f5 = TFile::Open("../goodB1Output/mar24_gammaTest1/liquid_standard_5cm.root", "READ");
	TTree * tA = (TTree*) f5->Get("ntup");

	TFile * f25 = TFile::Open("../goodB1Output/mar24_gammaTest1/liquid_standard_2-5cm.root", "READ");
	TTree * tB = (TTree*) f25->Get("ntup");


	int nbins = 250;
	float Elow = 0;
	float Ehigh = 5000; // in kev
	// TH1 * h10_edep = new TH1F("h10_edep", "10\% gold", nbins, Elow, Ehigh);
	// TH1 * h50_edep = (TH1*) h10_edep->Clone("h50_edep");
	// TH1 * h90_edep = (TH1*) h10_edep->Clone("h90_edep");

	// DSX::nameTH1(h10_edep, "Energy Deposited in CF4", "Energy (KeV)");

	// tA->Draw("fEdepInCyl_kev", "fEdepInCyl_kev>10", "hist");
	// tA->SetLineColor(2);
	// tA->Draw("fNPE", "fNPE>0", "hist");
	tB->SetMarkerColor(2);
	tB->SetMarkerStyle(8);
	tB->SetMarkerSize(0.1);
	tB->Draw("fmcNPE:fEdepInCyl_kev", "", "SCAT");

	// TCanvas * c1 = new TCanvas();
	// t10->Draw("fEdepInCyl_kev>>h10_edep", "", "hist");
	// t50->Draw("fEdepInCyl_kev>>h50_edep", "", "hist same");
	// t90->Draw("fEdepInCyl_kev>>h90_edep", "", "hist same");

	// h10_edep->SetLineColor(1);
	// h50_edep->SetLineColor(2);
	// h90_edep->SetLineColor(3);

	// h10_edep->GetYaxis()->SetRangeUser(0, 2000);

	// TLegend * leg1 = new TLegend();

	// leg1->SetHeader("#splitline{Comparing Gold fraction}{in active matrix}", "C");
	// leg1->AddEntry(h10_edep, "10\% Gold", "L");
	// leg1->AddEntry(h50_edep, "50\% Gold", "L");
	// leg1->AddEntry(h90_edep, "90\% Gold", "L");
	// leg1->Draw("same");

	// c1->Update();



	cout << "yo dog" << endl;
	return 0;
}