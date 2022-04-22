
#include "../scripts/adsxTools.C"

int sourceComp(){


  TFile * fa = TFile::Open("Cs137/iso_coll_run29A.root", "READ");
  TTree * ta = (TTree *)fa->Get("ntup");
  TFile * fa2 = TFile::Open("Cs137/iso_coll_run29B.root", "READ");
  TTree * ta2 = (TTree *)fa2->Get("ntup");
  TList * list = new TList;
  list->Add(ta);
  list->Add(ta2);
  
  TTree * tiso = TTree::MergeTrees(list);
  tiso->SetLineColor(4);
  tiso->SetLineWidth(3);
  tiso->SetName("tiso"); // iso tree
  
  TFile * fb = TFile::Open("Cs137/run29.root", "READ");
  TTree * tb = (TTree *)fb->Get("ntup"); // streight tree
  tb->SetLineColor(6);
  tb->SetLineWidth(10);
  
  ////---- energy deposited ---- ////
  TH1 * h_edepIso = new TH1F("h_edepIso", "", 100, 0, 1000);
  tiso->Draw("fEdepInCyl_kev>>h_edepIso", "fEdepInCyl_kev > 0", "goff");
  h_edepIso->Rebin(4);
  
  TH1 * h_edep = (TH1*) h_edepIso->Clone("h_edep");
  tb->Draw("fEdepInCyl_kev>>h_edep", "fEdepInCyl_kev > 0", "goff");
  h_edep->Scale(h_edepIso->GetEntries()/h_edep->GetEntries());

  new TCanvas();
  DSX::nameTH1(h_edep, "deposited energy", "energy (keV)");
  h_edep->Draw("hist E");
  h_edepIso->Draw("hist E same");
  
  ////---- photons ---- ////
  TH1 * hIsoNPE = new TH1F("hIsoNPE", "", 100, 0, 100);
  tiso->Draw("fNPE>>hIsoNPE", "fEdepInCyl_kev > 0 && fNPE > 0", "goff");
  // h_edepIso->Rebin(4);
  
  TH1 * hNPE = (TH1*) hIsoNPE->Clone("hNPE");
  tb->Draw("fNPE>>hNPE", "fEdepInCyl_kev > 0 && fNPE > 0", "goff");
  
  hNPE->Scale(hIsoNPE->GetEntries()/hNPE->GetEntries());

  auto c1 = new TCanvas();
  DSX::nameTH1(hNPE, "detected photons", "NPE");
  
  hNPE->SetLineColor(2);
  hNPE->Draw("PE");
  
  hIsoNPE->SetLineColor(3);
  hIsoNPE->Draw("E same");
  
  TLegend * leg = new TLegend();
  leg->SetHeader("-- Cs137 sources --", "C");
  leg->AddEntry(hNPE, "scaled, 2deg biased", "L");
  leg->AddEntry(hIsoNPE, "isotropic ", "L");
  leg->Draw("same");
  c1->SetLogy();

  // tb->Draw("fEdepInCyl_kev", "fEdepInCyl_kev > 0", "hist same");

  
  
  // tiso->Draw("fEdepInCyl_kev", "fEdepInCyl_kev > 0", "hist");
  // tb->Draw("fEdepInCyl_kev", "fEdepInCyl_kev > 0", "hist same");

  // TFile * fb = TFile::Open("Cs137/run29.root", "READ");
  // TTree * tb = (TTree *)fb->Get("ntup");
  // tb->SetLineColor(2);
  // 
  // TFile * fc = TFile::Open("Cs137/run44.root", "READ");
  // TTree * tc = (TTree *)fc->Get("ntup");
  // tc->SetLineColor(3);
  // 
  // TFile * fd = TFile::Open("Cs137/run-3.root", "READ");
  // TTree * td = (TTree *)fd->Get("ntup");
  // td->SetLineColor(4);

  
  // ta2->Draw("fEdepInCyl_kev", "fEdepInCyl_kev > 0", "hist same");
  // ta->Draw("fEdepInCyl_kev", "fEdepInCyl_kev > 0", "hist same");
  
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

