%module lazar
%{
#include "predictor.h"
%}

# "AM: use swig string conversion support"
%include "std_string.i"
typedef std::string String;
using namespace std;


# "AM: inline helper functions, passed both to swig and wrapper file"
%inline %{
shared_ptr<Out> getConsoleOut() {
    shared_ptr<Out> out (new ConsoleOut());
    return out;
};

shared_ptr<FeatMol <OBLazMol, ClassFeat, bool> > getClassMol (string smiles, shared_ptr<Out> out) {
    shared_ptr<FeatMol <OBLazMol, ClassFeat, bool> > cur_mol ( new FeatMol<OBLazMol, ClassFeat, bool>(0,"test structure",smiles,out) );
    return cur_mol;
};
%}

# "AM: Predictor"
template <class MolType, class FeatureType, class ActivityType>
class Predictor {
    public:
        # "Predictor constructor for LOO"
        Predictor(char* structure_file, char* act_file, char* feat_file, shared_ptr<Out> out);
        # "Predictor constructor for single SMILES prediction"
        Predictor(char* structure_file, char* act_file, char* feat_file, char* alphabet_file, shared_ptr<Out> out);
        # "Predictor constructor for batch prediction"
        Predictor(char* structure_file, char* act_file, char* feat_file, char* alphabet_file, char* input_file, shared_ptr<Out> out);
        # "predict a single smiles"
        void predict_smi(string smiles);
        # "batch prediction: predict witheld fold, i.e. compounds must occur in smi database, do make testset to generate fold tool."
        void predict_fold();
        # "batch predictions: predict arbitrary comps."
        void predict_file();
        # "leave one out crossvalidation"
        void loo_predict();
        # "predict a test structure"
        void predict(shared_ptr<FeatMol < MolType, FeatureType, ActivityType > > test_compound, bool recalculate, bool verbose);
        # "predict the activity act for the query structure"
        void knn_predict(shared_ptr<FeatMol < MolType, FeatureType, ActivityType > > test, string act, bool verbose);
        void print_neighbors(string act);
        
};
%template(ClassificationPredictor) Predictor<OBLazMol, ClassFeat, bool>;
