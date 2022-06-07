
int comp1(){
  
  TFile * fa = TFile::Open("Cs137/run14.root", "READ");
  TTree * ta = (TTree *)fa->Get("ntup");
  ta->SetLineColor(1);
  
  TFile * fa2 = TFile::Open("Co57/run14.root", "READ");
  TTree * ta2 = (TTree *)fa2->Get("ntup");
  ta2->SetLineColor(2);
  
  TFile * fb = TFile::Open("Cs137/run29.root", "READ");
  TTree * tb = (TTree *)fb->Get("ntup");
  tb->SetLineColor(2);
  
  TFile * fc = TFile::Open("Cs137/run44.root", "READ");
  TTree * tc = (TTree *)fc->Get("ntup");
  tc->SetLineColor(3);
  
  TFile * fd = TFile::Open("Cs137/run-3.root", "READ");
  TTree * td = (TTree *)fd->Get("ntup");
  td->SetLineColor(4);
  
  int nbins = 200;
  float Elow = 0;
  float Ehigh = 2000; // in kev
  
  ta2->Draw("fEdepInCyl_kev", "fEdepInCyl_kev > 0", "hist");
  ta->Draw("fEdepInCyl_kev", "fEdepInCyl_kev > 0", "hist same");
  
  // tb->Draw("fEdepInCyl_kev", "fEdepInCyl_kev > 0", "hist same");
  // tc->Draw("fEdepInCyl_kev", "fEdepInCyl_kev > 0", "hist same");
  // td->Draw("fEdepInCyl_kev", "fEdepInCyl_kev > 0", "hist same");
  
  // string var = "fEdepInCyl_kev";
  
  // ta->Draw("fNPE", "fEdepInCyl_kev > 0", "hist");
  // tb->Draw("fNPE", "fEdepInCyl_kev > 0", "hist same");
  // tc->Draw("fNPE", "fEdepInCyl_kev > 0", "hist same");
  // td->Draw("fNPE", "fEdepInCyl_kev > 0", "hist");
  
  // TH1 * h10_edep = new TH1F("h10_edep", "10\% gold", nbins, Elow, Ehigh);
  // TH1 * h50_edep = (TH1*) h10_edep->Clone("h50_edep");


  // DSX::nameTH1(h10_edep, "Energy Deposited in CF4", "Energy (KeV)");

  // tA->Draw("fEdepInCyl_kev", "fEdepInCyl_kev>10", "hist");
  // tA->SetLineColor(2);
  // tA->Draw("fNPE", "fNPE>0", "hist");
  // tB->SetMarkerColor(2);
  // tB->SetMarkerStyle(8);
  // tB->SetMarkerSize(0.1);
  // tB->Draw("fmcNPE:fEdepInCyl_kev", "", "SCAT");
    
  return 1;
}

