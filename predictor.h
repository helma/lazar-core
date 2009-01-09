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


#include "feature-generation.h"
#include "activity-db.h"
#include "model.h"
#include "time.h"
#include "boost/smart_ptr.hpp"

using namespace std;
using namespace OpenBabel;
using namespace boost;

extern bool kernel;
extern bool quantitative;

//! make predictions from training data (structures, activities, features)
template <class MolType, class FeatureType, class ActivityType>
class Predictor {

	public:

		typedef FeatMol < MolType, FeatureType, ActivityType > * MolRef ;
		typedef Feature<OBLinFrag> * OBLinFragRef;
		typedef Feature<FeatureType> * FeatRef;

	private:

		//! feature generation for new structures
		shared_ptr<FeatGen <MolType, FeatureType, ActivityType> > feat_gen;
		//! training structures
		shared_ptr<ActMolVect <MolType, FeatureType, ActivityType> > train_structures;
		//! test structures for batch predictions
		shared_ptr<MolVect <MolType, FeatureType, ActivityType> > test_structures;
		//! model
		shared_ptr<MetaModel<MolType, FeatureType, ActivityType> > model;


		//! neighbors for the prediction of the current query structure
		vector<MolRef> neighbors;
		//! input file with activities
		char* a_file;
		//! make leave-one-out crossvalidation?
		bool loo;
		//! output object
		Out* out;

	public:

		//! Predictor constructor for LOO
		Predictor(char * structure_file, char * act_file, char * feat_file, Out * out): a_file(NULL), loo(false), out(out) {
			train_structures.reset( new ActMolVect <MolType, FeatureType, ActivityType>(act_file, feat_file, structure_file, out) );
            if (kernel) model.reset( new KernelModel<MolType, FeatureType, ActivityType>(out) );
            else model.reset( new Model<MolType, FeatureType, ActivityType>(out) );
		};

		//! Predictor constructor for single SMILES prediction
		Predictor(char * structure_file, char * act_file, char * feat_file, char * alphabet_file, Out* out): a_file(alphabet_file), loo(false), out(out){
			train_structures.reset( new ActMolVect <MolType, FeatureType, ActivityType>(act_file, feat_file, structure_file, out) );
            if (kernel) model.reset( new KernelModel<MolType, FeatureType, ActivityType>(out ));
            else model.reset(new Model<MolType, FeatureType, ActivityType>(out));
		}

		//! Predictor constructor for batch prediction
		Predictor(char * structure_file, char * act_file, char * feat_file, char * alphabet_file, char * input_file, Out* out): a_file(alphabet_file), loo(false), out(out){
			train_structures.reset( new ActMolVect <MolType, FeatureType, ActivityType>(act_file, feat_file, structure_file, out) );
			test_structures.reset( new MolVect <MolType, FeatureType, ActivityType>(input_file, out) );
            if (kernel) model.reset( new KernelModel<MolType, FeatureType, ActivityType>(out) );
            else model.reset( new Model<MolType, FeatureType, ActivityType>(out) );
		}


		//! predict a single smiles
		void predict_smi(string smiles);

		//! batch predictions
		void predict_ext();
		void predict_file();

		//! leave one out crossvalidation
		void loo_predict();

		//! predict a test structure
		void predict(MolRef test_compound, bool recalculate, bool verbose);

		//! predict the activity act for the query structure
		void knn_predict(MolRef test, string act, bool verbose);

		//! determine similarities with for the query structure
		void similarities(MolRef test_compound);

		void print_neighbors(string act);

		//! set the output object (e.g. switch between console and socket)
		void set_output(Out * newout);

		//! match features (SMARTS) from a file
		void match_file_smarts(char * file);

		//! apply y-scrambling (aka response permutation testing, see Eriksson et al. 2003)
		vector<map<string, vector<ActivityType> > > y_scrambling();

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::predict_ext() {

		typename vector<MolRef>::iterator cur_dup;

		MolRef cur_mol;
		int test_size = test_structures->get_size();


		for (int n = 0; n < test_size; n++) {
			cur_mol = test_structures->get_compound(n);
			//delete feat_gen;

			vector<MolRef> duplicates = train_structures->remove_duplicates(cur_mol);
			if (duplicates.size()) {
				*out << int(duplicates.size()) << " instances of " << cur_mol->get_smiles() << " removed from the training set!\n";
				out->print_err();
			}

		}

		for (int n = 0; n < test_size; n++) {
			cur_mol = test_structures->get_compound(n);
			//delete feat_gen;
			feat_gen.reset( new FeatGen <MolType, FeatureType, ActivityType>(a_file, train_structures.get(), cur_mol,out) );
			feat_gen->generate_linfrag(train_structures.get(),cur_mol);

			*out << "Predicting external test id " << cur_mol->get_id() << endl;
			out->print_err();

			// recalculate frequencies and and significance only for the first time
			if (n == 0)
				this->predict(cur_mol, true);
			else
				this->predict(cur_mol, false);

		}

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::predict_file() {

		bool recalculate = true;
		typename vector<MolRef>::iterator cur_dup;

		MolRef cur_mol;
		int test_size = test_structures->get_size();

		for (int n = 0; n < test_size; n++) {

			cur_mol = test_structures->get_compound(n);
			//delete feat_gen;
			feat_gen.reset(new FeatGen <MolType, FeatureType, ActivityType>(a_file, train_structures.get(), cur_mol,out));
			feat_gen->generate_linfrag(train_structures.get(),cur_mol);

			//cur_mol->print();

			// check if the compound is already in the database

			*out << "Looking for " << cur_mol->get_smiles() << " in the training set\n";
			out->print_err();

			vector<MolRef> duplicates = train_structures->remove_duplicates(cur_mol);

			if (duplicates.size() > 1) {
				*out << int(duplicates.size()) << " instances of " << cur_mol->get_smiles() << " in the training set!\n";
				out->print_err();
			}

			// recalculate frequencies and and significance only if necessary

			if (n == 0)
				this->predict(cur_mol, true, true);
			else if (duplicates.size() > 0) {
				this->predict(cur_mol, true, true);
				recalculate = true;
			}
			else if (recalculate) {
				this->predict(cur_mol, true, true);
				recalculate = false;
			}
			else
				this->predict(cur_mol, false, true);

			// restore duplicates for batch predictions

			if (duplicates.size() >= 1) {
				for (cur_dup=duplicates.begin(); cur_dup != duplicates.end(); cur_dup++) {
					(*cur_dup)->restore();
				}
			}
		}

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::loo_predict() {

	loo = true;
	MolRef cur_mol;
	typename vector<MolRef>::iterator cur_dup;

   
	clock_t t1 = clock();
	cerr << "Precomputing significance values... ";

	// MG : precompute
	vector<ActivityType> activity_values;
	vector<string> activity_names = train_structures->get_activity_names();
	typename vector<string>::iterator cur_act;

	for (cur_act = activity_names.begin(); cur_act != activity_names.end(); cur_act++) {

		activity_values = train_structures->get_activity_values(*cur_act);
		train_structures->precompute_feature_significance(*cur_act, activity_values);

	}
	// MG

	clock_t t2 = clock();
	cerr << "done (" << (float)(t2-t1)/CLOCKS_PER_SEC << "sec)!" << endl;

	for (int n = 0; n < train_structures->get_size(); n++) {

		t1 = clock();

		cur_mol = train_structures->get_compound(n);

		// make query compound unavailable as train structure in this round
		*out << "Looking for " << cur_mol->get_smiles() << " in the training set\n";
		out->print_err();
		vector<MolRef> duplicates = train_structures->remove_duplicates(cur_mol);
		if (duplicates.size() > 1) {
			*out << duplicates.size() << " instances of " << cur_mol->get_smiles() << " in the training set!\n";
			out->print_err();
		}
		// predict by recalculating significance values
		this->predict(cur_mol,true,false);

        // recover query compound as train structure for the next round
		if (duplicates.size() >= 1) {
			for (cur_dup=duplicates.begin(); cur_dup != duplicates.end(); cur_dup++) {
				(*cur_dup)->restore();
			}
		}

		t2 = clock();
		static float avg_s = 0;
		float s = (float)(t2-t1)/CLOCKS_PER_SEC;
		avg_s = (avg_s * n + s) / float(n+1);

		cerr << "Prediction " << n+1 << "/" << train_structures->get_size() << " of structure " <<cur_mol->get_id() <<
		  " took " << s << " sec (avg is " << avg_s << " sec)" << endl;
	}


};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::predict_smi(string smiles) {

		bool recalculate = true;
		vector<MolRef> duplicates ;
		typename vector<MolRef>::iterator cur_dup;

        shared_ptr<FeatMol <MolType, FeatureType, ActivityType> > cur_mol ( new FeatMol<MolType, FeatureType, ActivityType>(0,"test structure",smiles,out) );

		*out << "Looking for " << cur_mol->get_smiles() << " in the training set\n";
		out->print_err();
		duplicates = train_structures->remove_duplicates(cur_mol.get());

		//delete feat_gen;
        feat_gen.reset( new FeatGen <MolType, FeatureType, ActivityType>(a_file, train_structures.get(), cur_mol.get(),out)) ;
		feat_gen->generate_linfrag(train_structures.get(),cur_mol.get());

		if (duplicates.size() > 1) {
			*out << int(duplicates.size()) << " instances of " << cur_mol->get_smiles() << " in the training set!\n";
			out->print_err();
		}
		else if (duplicates.size() > 0) {
			this->predict(cur_mol.get(), true, true);
		}
		else if (recalculate) {
			this->predict(cur_mol.get(), true, true);
		}
		else
			this->predict(cur_mol.get(), false, true);

		// restore duplicates for batch predictions

		if (duplicates.size() >= 1) {
			for (cur_dup=duplicates.begin(); cur_dup != duplicates.end(); cur_dup++) {
				(*cur_dup)->restore();
			}
		}

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::predict(MolRef test, bool recalculate, bool verbose=true) {

	vector<ActivityType> activity_values;
	vector<string> activity_names = train_structures->get_activity_names();
	typename vector<string>::iterator cur_act;

	// determine common features in the training set
	train_structures->common_features(test);

	for (cur_act = activity_names.begin(); cur_act != activity_names.end(); cur_act++) {


		if (!loo || test->db_act_available(*cur_act)) {	// make loo predictions only for activities with measured values

			*out << "---\n";

			if (recalculate) {

                if (!loo) {
    				activity_values = train_structures->get_activity_values(*cur_act);
	    			train_structures->feature_significance(*cur_act, activity_values);	// AM: feature significance
                }

                // MG
                else {
                    typename vector<FeatRef>::iterator cur_feat;
                    vector<ActivityType> tmp_activities;

                    tmp_activities = test->get_act(*cur_act);
                    if (tmp_activities.size() > 1) {
                        fprintf(stderr, "Current test structure has more than one activity value");
                        exit(1);
                    }
                    ClassFeat::set_cur_str_active( *tmp_activities.begin() );

                    // label features that occur in current test structure
                    vector<FeatRef> test_features = test->get_features();
                    for (cur_feat=test_features.begin(); cur_feat!=test_features.end(); cur_feat++){
                        (*cur_feat)->set_cur_feat_occurs( true );
                    }
				}
			}
			else {
				*out << "Significances for " << *cur_act << " not recalculated.\n";
				out->print_err();
			}

			train_structures->relevant_features(test, *cur_act);
			this->knn_predict(test,*cur_act);
            *out << "\n";
			out->print();


			// MG: remove label that feature occurs in current test structure
			typename vector<FeatRef>::iterator cur_feat;
			vector<FeatRef> test_features = test->get_features();
			for (cur_feat=test_features.begin(); cur_feat!=test_features.end(); cur_feat++){
				(*cur_feat)->set_cur_feat_occurs( false );
			}
			// MG
		}

		else cerr << "test db act not avail" << endl;
	}

};


template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::similarities(MolRef test) {

	vector<MolRef> compounds  = train_structures->get_compounds();
	typename vector<MolRef>::iterator cur_mol;

	for (cur_mol=compounds.begin();cur_mol!=compounds.end();cur_mol++) {
		(*cur_mol)->calc_similarity(test);
	}

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::knn_predict(MolRef test, string act, bool verbose=true)  {

	// determine neighbors
	train_structures->get_neighbors(act, &neighbors);

	// determine and print
	train_structures->determine_unknown(act, test);

	// calculate and print predicition
    test->print();
    test->print_db_activity(act,loo);
	model->calculate_prediction(test, &neighbors, act);
	*out << "endpoint: '" << act << "'\n";
	out->print();

    // print neighbors
    if (verbose) {
        *out << "neighbors:\n";
        this->print_neighbors(act);
        *out << "features:\n";
        test->print_features(act);
	    out->print();
    }
	*out << "unknown_features:\n";
	test->print_unknown(act);
	out->print();

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::print_neighbors(string act) {

	int n;
	typename vector<MolRef>::iterator cur_n;

	sort(neighbors.begin(),neighbors.end(),greater_sim<MolType,FeatureType,ActivityType>());

	if (neighbors.size()>0) {

		n = 0;
		for (cur_n = neighbors.begin(); (cur_n != neighbors.end()); cur_n++) {
			(*cur_n)->print_neighbor(act);
			n++;
		}
	}

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::set_output(Out * newout) {

	out = newout ;
	int train_size = train_structures->get_size();
	model->set_output(out);

	for (int n = 0; n < train_size; n++) {
		train_structures->get_compound(n)->set_output(out);
	}

};

template <class MolType, class FeatureType, class ActivityType>
vector<map<string, vector<ActivityType> > > Predictor<MolType, FeatureType, ActivityType>::y_scrambling() {

	map<string, vector<ActivityType> > val;
	typename vector<map<string, vector<ActivityType> > >::iterator act_it;
	vector<MolRef> tc;
	typename vector<MolRef>::iterator tc_it;

	// gather activities
	vector<map<string, vector<ActivityType> > > act_avail;
	vector<map<string, vector<ActivityType> > > act_ori;

	tc=train_structures->get_compounds();
	for (tc_it = tc.begin(); tc_it != tc.end(); tc_it++) {
		act_avail.push_back((*tc_it)->get_activities());
	}
	act_ori.clear();
	act_ori.assign(act_avail.begin(), act_avail.end());

	// draw random activity without replacement
	for (int n=0; n<train_structures->get_size(); n++) {
		srand(static_cast<unsigned int>(clock()));
		double dpos = double(rand()) / (double(RAND_MAX) + 1.0);
		int ipos = (int) (dpos*act_avail.size());
		val.clear();
		val = act_avail.at(ipos);
		for (act_it = act_avail.begin(); act_it != act_avail.end(); act_it++) {
			if (*(act_it) == val) break;
		}

		act_avail.erase(act_it);

		train_structures->get_compound(n)->replace_activities(val);
	}

	return act_ori;

}


