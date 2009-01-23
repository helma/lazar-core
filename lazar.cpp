/* Copyright (C) 2005  Christoph Helma <helma@in-silico.de>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/


#include <getopt.h>
#include <memory>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "boost/smart_ptr.hpp"
#include "predictor.h"

using namespace std;
using namespace boost;

float sig_thr = 0.9;
bool kernel = false;
bool quantitative = false;

// daemon shutdown
void shutdown(int s) {
    cerr << "Stopping daemon. [" << s << "]\n";
    exit(0);
};

void remove_dos_cr(string* str) {
    string nl = "\r";
    for (string::size_type i = str->find(nl); i!=string::npos; i=str->find(nl)) str->erase(i,1); // erase dos cr
}

//! lazar predictions
int main(int argc, char *argv[], char *envp[]) {

    int status = 0;
    int c;
    bool s_file = false;
    bool t_file = false;
    bool f_file = false;
    bool a_file = false;
    bool i_file = false;
    bool loo = false;
    bool daemon = false;
    char* smi_file = NULL;
    char* train_file = NULL;
    char* feature_file = NULL;
    char* alphabet_file = NULL;
    char* input_file = NULL;

    int port = 0;
    string smiles;

    shared_ptr< Predictor<OBLazMol,ClassFeat,bool> > train_set_c;
    shared_ptr< Predictor<OBLazMol,RegrFeat,float> > train_set_r;


    // argument parsing
    while ((c = getopt(argc, argv, "rkxhs:t:f:a:i:p:m:")) != -1) {
        switch (c) {
        case 's':
            smi_file = optarg;
            s_file = true;
            break;
        case 't':
            train_file = optarg;
            t_file = true;
            break;
        case 'f':
            feature_file = optarg;
            f_file = true;
            break;
        case 'a':
            alphabet_file = optarg;
            a_file = true;
            break;
        case 'i':
            input_file = optarg;
            i_file = true;
            break;
        case 'p':
            port = atoi(optarg);
            daemon = true;
            break;
        case 'h':
            status = 1;
            break;
        case 'x':
            loo = true;
            break;
        case 'r':
            quantitative = true;
            break;
        case 'k':
            kernel = true;
            break;
        case 'm':
            sig_thr = atof(optarg);
            if (!quantitative) status = 1;
            break;
        case ':':
            status = 1;
            break;
        case '?':
            status = 1;
            break;
        }
    }

    // structures, activities and features are always required
    if (!s_file | !t_file | !f_file)
        status = 1;

    // no alphabet required for LOO
    if (!loo & !a_file)
        status = 1;

    // no input allowed for daemon
    if (daemon & (loo | i_file | (optind < argc)))
        status = 1;

    // smiles input required for command line predictions
    if (!daemon & !loo & !i_file & (optind == argc) )
        status = 1;

    // print usage and examples for incorrect input
    if (status)  {
        cerr << "usage: " << argv[0] << " -s smiles_structures -t training_set -f feature_set [-r [-m significance_threshold]] [-k] [-a alphabet_file [\"smiles_string\"|-i test_set_file|-p port]|-x]\n";
        cerr << "\nexamples:\n";
        cerr << "\t# leave-one-out crossvalidation\n\t" << argv[0] <<  " -s smiles_structures -t training_set -f feature_set -x [-r] [-k]\n";
        cerr << "\t# predict smiles_string\n\t" << argv[0] <<  " -s smiles_structures -t training_set -f feature_set -a alphabet_file \"smiles_string\" [-r] [-k]\n";
        cerr << "\t# predict test_set_file\n\t" << argv[0] <<  " -s smiles_structures -t training_set -f feature_set -a alphabet_file -i test_set_file [-r] [-k]\n";
        cerr << "\t# start daemon on port\n\t" << argv[0] <<  " -s smiles_structures -t training_set -f feature_set -a alphabet_file -p port [-r] [-k]\n";
        return(status);
    }

    //shared_ptr<Out> out(new ConsoleOut());         // write to STDOUT/STDERR
    shared_ptr<Out> out(new ConsoleOut());         // write to STDOUT/STDERR

    obErrorLog.StopLogging();
   
    // initialize R
    cerr << "Initializing R environment...";

    char *R_argv[] = { (char*)"REmbeddedPostgres", (char*)"--gui=none", (char*)"--silent", (char*)"--no-save"};
    int R_argc = sizeof(R_argv)/sizeof(R_argv[0]);

    init_R(R_argc, R_argv);
    R_exec("library", mkString("kernlab"));
    cerr << "done!" << endl;

    // restore SIGINT handling (damaged by R)
    struct sigaction sa;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = SIG_DFL;
    sigaction (SIGINT, &sa, NULL);

    // start predictions
    if (!daemon) {            // keep writing to STDOUT/STDERR

        if (loo) {            // LOO crossvalidation
            out->print();
            if (!quantitative) {
                train_set_c.reset( new Predictor<OBLazMol,ClassFeat,bool>(smi_file, train_file, feature_file, out) );
                train_set_c->loo_predict();
            }
            else {
                train_set_r.reset( new Predictor<OBLazMol,RegrFeat,float>(smi_file, train_file, feature_file, out) );
                train_set_r->loo_predict();
            }
            out->print();

        }

        else {

            if (!i_file) {        // read smiles from command line
                smiles = argv[optind];
                optind++;
                out->print();
                if (!quantitative) {
                    train_set_c.reset ( new Predictor<OBLazMol,ClassFeat,bool>(smi_file, train_file, feature_file, alphabet_file,out) );
                    train_set_c->predict_smi(smiles); // AM: start SMILES -> predictor.h
                }
                else {
                    train_set_r.reset ( new Predictor<OBLazMol,RegrFeat,float>(smi_file, train_file, feature_file, alphabet_file,out) );
                    train_set_r->predict_smi(smiles); // AM: start SMILES -> predictor.h
                }
            }

            else {            // read input file batch predictions
                out->print();
                if (!quantitative) {
                    train_set_c.reset( new Predictor<OBLazMol,ClassFeat,bool>(smi_file, train_file, feature_file, alphabet_file, input_file, out) );
                    train_set_c->predict_file(); // AM: start SMILES -> predictor.h
                }
                else {
                    train_set_r.reset ( new Predictor<OBLazMol,RegrFeat,float>(smi_file, train_file, feature_file, alphabet_file, input_file, out) );
                    train_set_r->predict_file(); // AM: start SMILES -> predictor.h
                }
                out->print();
            }
        }
    }

    else {                // read/write from socket

        pid_t pid, sid;         // process ID and Session ID

        pid = fork();         // fork
        if (pid < 0)
            exit(EXIT_FAILURE);
        if (pid > 0)
            exit(EXIT_SUCCESS);
        sid = setsid();         // start child and store child id
        if (sid < 0) exit(EXIT_FAILURE);

        if (!quantitative) train_set_c.reset( new Predictor<OBLazMol,ClassFeat,bool>(smi_file, train_file, feature_file, alphabet_file, out) );
        else train_set_r.reset( new Predictor<OBLazMol,RegrFeat,float>(smi_file, train_file, feature_file, alphabet_file, out) );

        string tmp;
        signal(SIGTERM, shutdown);

        try {                 // start daemon

            ServerSocket server( port );    // create the listening socket

            while ( true ) {

                ServerSocket socket;      // conversational socket
                server.accept ( socket );   // wait for a client connection
                out.reset( new SocketOut(&socket) ); // create a new output object

                if (!quantitative)
                    train_set_c->set_output(out);       // set new output
                else
                    train_set_r->set_output(out);       // set new output

                cerr << "client accepted\n";

                try {

                    while ( true ) {

                        socket >> tmp;            // read input line
                        istringstream iss(tmp);
                        smiles = "";
                        getline(iss, smiles);

                        if (!smiles.empty()) {
                            std::string::iterator p;
                            for (p = smiles.end(); p!=smiles.begin() && isspace(*--p););
                            if (!isspace(*p)) p++;
                            smiles.erase(p, smiles.end());
                        }

                        out->print();

                        if (!quantitative) train_set_c->predict_smi(smiles);    // predict
                        else train_set_r->predict_smi(smiles);    // predict

                        server.remove ( socket ); // disconnect
                    }
                }
                catch (...) {         // no client connection
                    cerr << "client removed.\n";
                }
            }
        }
        catch (...) {
            cerr << "Exception was caught.\n";
            shutdown(0);
        }
    }

    return (0);
}
