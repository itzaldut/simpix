#include "TROOT.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TASImage.h"
#include "TApplication.h"
#include "TSystem.h"

#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>

using namespace std;

inline double colorDist(UInt_t a, UInt_t b) {
    int ra = (a >> 16) & 0xff, ga = (a >> 8) & 0xff, ba = (a) & 0xff;
    int rb = (b >> 16) & 0xff, gb = (b >> 8) & 0xff, bb = (b) & 0xff;
    double dr = ra - rb, dg = ga - gb, db = ba - bb;
    return sqrt(dr*dr + dg*dg + db*db);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cout << "Usage: simpix image_src image_tgt <out=out.png>\n";
        return 1;
    }

    TString srcFile = argv[1];
    TString tgtFile = argv[2];
    TString outFile = (argc > 3 ? argv[3] : "out.png");

    cout << "Source: " << srcFile << " Target: " << tgtFile << "\n";

    TApplication app("app", &argc, argv);

    TASImage *src = new TASImage(srcFile.Data());
    TASImage *tgt = new TASImage(tgtFile.Data());
    assert(src->GetWidth() == tgt->GetWidth() &&
           src->GetHeight() == tgt->GetHeight());

    int W = src->GetWidth(), H = src->GetHeight();
    long N = W * H;
    cout << "Image size: " << W << "x" << H << " total pix = " << N << "\n";

    TASImage *out = new TASImage(*src);
    UInt_t* outPix = out->GetArgbArray();
    UInt_t* tgtPix = tgt->GetArgbArray();

    auto energy = [&](void) {
        double E = 0.0;
        for (int i = 0; i < N; i++)
            E += colorDist(outPix[i], tgtPix[i]);
        return E;
    };

    double currE = energy();
    cout << "Initial energy: " << currE << "\n";

    mt19937_64 rng(random_device{}());
    uniform_int_distribution<int> distIdx(0, N - 1);
    uniform_real_distribution<double> distU(0.0, 1.0);

    double T = 1e3;
    double Tmin = 1e-3;
    double alpha = 0.99;

    auto t_start = chrono::high_resolution_clock::now();

    while (T > Tmin) {
        int i = distIdx(rng);
        int j = distIdx(rng);
        if (i == j) continue;

        double before = colorDist(outPix[i], tgtPix[i]) +
                        colorDist(outPix[j], tgtPix[j]);
        double after  = colorDist(outPix[j], tgtPix[i]) +
                        colorDist(outPix[i], tgtPix[j]);
        double dE = after - before;

        if (dE < 0 || exp(-dE / T) > distU(rng)) {
            swap(outPix[i], outPix[j]);
            currE += dE;
        }

        T *= alpha;
    }

    auto t_end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = t_end - t_start;

    cout << "Final energy: " << currE << "\n";
    cout << "Annealing time: " << elapsed.count() << " seconds\n";

    TCanvas *c1 = new TCanvas("c1", "simpix output", W, H);
    c1->Divide(2, 2);
    c1->cd(1); src->Draw("X");
    c1->cd(2); tgt->Draw("X");
    c1->cd(3); out->Draw("X");
    c1->Print("collage.png");

    out->WriteImage(outFile.Data());
    cout << "Done. Output saved to " << outFile << "\n";
    return 0;
}

