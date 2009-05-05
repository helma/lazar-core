# "BEGIN DECLARATION SECTION"
%module lazar
%{
#include "predictor.h"
%}
# "use swig string conversion support"
%include "std_string.i"
typedef std::string String;
using namespace std;
# "use swig STL conversion support"
%include "std_vector.i"
# "END DECLARATION SECTION"



# "BEGIN GLOBAL VARIABLES"
extern float sig_thr;
extern bool kernel;
extern bool quantitative;
# "END GLOBAL VARIABLES"



# "BEGIN INLINE SECTION"
# "inline helper functions, passed both to swig and wrapper file"
# "needed to make smart pointers from target language"
%inline %{
shared_ptr<Out> getConsoleOut() {
    shared_ptr<Out> out (new ConsoleOut());
    return out;
};
shared_ptr<Out> getStringOut() {
    shared_ptr<Out> out (new StringOut());
    return out;
};
shared_ptr<FeatMol <OBLazMol, ClassFeat, bool> > getClassMol (string smiles, shared_ptr<Out> out) {
    shared_ptr<FeatMol <OBLazMol, ClassFeat, bool> > cur_mol ( new FeatMol<OBLazMol, ClassFeat, bool>(0,"test structure",smiles,out) );
    return cur_mol;
};
%}
# "END INLINE SECTION"



# "BEGIN WRAPPING SECTION"
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
        string get_yaml();
};
%template(ClassificationPredictor) Predictor<OBLazMol, ClassFeat, bool>;
%template(RegressionPredictor) Predictor<OBLazMol, RegrFeat, float>;

class StringOut {
    public:
        string get_yaml();
};
# "END WRAPPING SECTION"
