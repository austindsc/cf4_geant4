
#include "../scripts/adsxTools.C"

int compCoHeight(){

  vector<string> files = {"run44", "run29", "run14", "run-3"};
  string home = "/Users/austindestcroix/Desktop/school_holders/projects/sbc/geant4_cf4/goodB1Output/b1_april11/Co57/";

  TList * tlist = new TList;
  for (std::vector<string>::iterator i = files.begin(); i != files.end(); ++i){
    // cout << home+(*i) << endl;
    TFile * f = TFile::Open((home+(*i)+".root").c_str(), "READ");
    TTree * t = (TTree *)f->Get("ntup");
    tlist->Add(t);
  }
  
  TList * hnpeList = new TList;
  TTree * harray[4];
  for (int i = 0; i < 4; ++i){
    TTree * t = ((TTree*)tlist->At(i));
    // TH1 * h = new TH1F(files[i].c_str(), "", 100, 0, 20);
    t->SetLineColor(i+1);
    harray[i] = t;
    cout << t->GetEntries() << endl;
  }

  ((TTree*)tlist->At(0))->Show(1);
  // return 1;

  // ((TTree*)tlist->At(0))->Draw("fEdepInCyl_kev", "fEdepInCyl_kev>0", "hist");

  auto c0 = new TCanvas();
  gStyle->SetOptStat(0);

  TH1 * he0 = new TH1F("heFar", "", 100, 0, 150);
  TH1 * he1 = (TH1*) he0->Clone("heMid");
  TH1 * he2 = (TH1*) he0->Clone("heClose");
  harray[0]->Draw("fEdepInCyl_kev>>heFar", "fEdepInCyl_kev>0", "goff");
  harray[1]->Draw("fEdepInCyl_kev>>heMid", "fEdepInCyl_kev>0", "goff");
  he1->Scale(1./10.);
  harray[2]->Draw("fEdepInCyl_kev>>heClose", "fEdepInCyl_kev>0", "goff");

  he0->SetTitle("Gamma Energy Deposited in liquid");
  he0->GetXaxis()->SetTitle("Energy Deposited (keV)");
  he0->GetYaxis()->SetTitle("Counts");

  he0->SetLineColor(2);
  he0->Draw("hist");

  he1->SetLineColor(3);
  he1->Draw("hist same");

  he2->SetLineColor(4);
  he2->Draw("hist same");

  he0->GetYaxis()->SetRangeUser(0, 1200);

  TLegend * leg = new TLegend();
  leg->SetTextSize(0.04);
  leg->SetHeader("-- Co Source Height --", "C");
  leg->AddEntry(he0, "4.4 cm", "L");
  leg->AddEntry(he1, "2.9 cm", "L");
  leg->AddEntry(he2, "1.4 cm ", "L");
  leg->Draw("same");
  // c0->cd(2)->SetLogy();



  auto c1 = new TCanvas();
  
  TH1 * hn0 = new TH1F("hnFar", "", 30, 0, 30);
  TH1 * hn1 = (TH1*) hn0->Clone("hnMid");
  TH1 * hn2 = (TH1*) hn0->Clone("hnClose");

  ((TTree*)tlist->At(2))->Draw("fNPE>>hnClose", "fNPE>0", "goff");
  ((TTree*)tlist->At(1))->Draw("fNPE>>hnMid", "fNPE>0", "goff");
  ((TTree*)tlist->At(0))->Draw("fNPE>>hnFar", "fNPE>0", "goff");

  // cout << hn0->GetXaxis()->GetBinLowEdge(2) << endl;
  // cout << hn0->Integral(1, 1) << endl;
  // return 1;
  hn0->Scale(1./hn0->Integral(1, 100));
  hn1->Scale(1./hn1->Integral(1, 100));
  hn2->Scale(1./hn2->Integral(1, 100));

  hn2->SetTitle("Simulation: Detected Photons");
  hn2->GetXaxis()->SetTitle("Photons detected (PE)");
  hn2->GetYaxis()->SetTitle("Counts");

  hn2->SetLineColor(2);
  hn2->Draw("hist");

  hn1->SetLineColor(3);
  hn1->Draw("hist same");

  hn0->SetLineColor(4);
  hn0->Draw("hist same");

  // hn0->GetYaxis()->SetRangeUser(1, 1200);

  TLegend * leg1 = new TLegend();
  leg1->SetTextSize(0.04);
  leg1->SetHeader("-- Co Source Height --", "C");
  leg1->AddEntry(hn0, "4.4 cm", "L");
  leg1->AddEntry(hn1, "2.9 cm", "L");
  leg1->AddEntry(hn2, "1.4 cm ", "L");
  leg1->Draw("same");
  // c0->cd(2)->SetLogy();

  // tiso->Draw("fNPE>>hIsoNPE", "fEdepInCyl_kev > 0 && fNPE > 0", "goff");



    
  return 1;
}

