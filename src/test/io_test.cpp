#include <gtest/gtest.h>
#include <tfhe_io.h>
#include <set>
#include "polynomials_arithmetic.h"

using namespace std;

namespace {

    const LweParams* lweparams500 = new_LweParams(500,0.1,0.3);
    const LweParams* lweparams120 = new_LweParams(120,0.1,0.3);
    const set<const LweParams*> allparams = { lweparams120, lweparams500 };

    const TLweParams* tlweparams1024_1 = new_TLweParams(1024,1,0.1,0.3);
    const TLweParams* tlweparams153_2 = new_TLweParams(153,2,0.1,0.3);
    const set<const TLweParams*> allparams_tlwe = { tlweparams153_2, tlweparams1024_1 };

    const TGswParams* tgswparams1024_1 = new_TGswParams(3,15,tlweparams1024_1);
    const set<const TGswParams*> allparams_tgsw = { tgswparams1024_1 };

    const TFheGateBootstrappingParameterSet* gbp1 = new TFheGateBootstrappingParameterSet(6,2,lweparams120,tgswparams1024_1);
    const set<const TFheGateBootstrappingParameterSet*> allgbp = { gbp1 };

    //generate a random lwekey
    LweKey* new_random_lwe_key(const LweParams* params) {
	const int n = params->n;
	LweKey* key = new_LweKey(params);
	for (int i=0; i<n; i++) key->key[i]=rand()%2;
	return key;
    }

    //generate a random tlwekey
    TLweKey* new_random_tlwe_key(const TLweParams* params) {
	const int N = params->N;
	const int k = params->k;
	TLweKey* key = new_TLweKey(params);
	for (int i=0; i<k; i++) 
	    for (int j=0; j<N; j++)
		key->key[i].coefs[j]=rand()%2;
	return key;
    }

    //generate a random tgswkey
    TGswKey* new_random_tgsw_key(const TGswParams* params) {
	const int N = params->tlwe_params->N;
	const int k = params->tlwe_params->k;
	TGswKey* key = new_TGswKey(params);
	for (int i=0; i<k; i++) 
	    for (int j=0; j<N; j++)
		key->key[i].coefs[j]=rand()%2;
	return key;
    }

    
    //generate a random ks
    void random_ks_key(LweKeySwitchKey* key) {
        const int N = key->n;
        const int t = key->t;
        const int base = key->base;
	const int n = key->out_params->n;
	const int length = N*t*base;
	double variance = rand()/double(RAND_MAX);
        LweSample* begin = key->ks0_raw;
	LweSample* end = begin+length;
	for (LweSample* it=begin; it!=end; ++it) {
	    for (int j=0; j<n; j++) it->a[j]=rand();
            it->b=rand();
            it->current_variance=variance;
        }
    }

    //generate a random ks
    LweKeySwitchKey* new_random_ks_key(int N, int t, int basebit, const LweParams* out_params) {
	LweKeySwitchKey* key = new_LweKeySwitchKey(N,t,basebit,out_params);
        random_ks_key(key);
	return key;
    }

    
    //generate a random ks
    LweBootstrappingKey* new_random_bk_key(int ks_t, int ks_basebit, const LweParams* in_out_params, const TGswParams* bk_params) {
	const int n = in_out_params->n;
        const int kpl = bk_params->kpl;
        const int k = bk_params->tlwe_params->k;
        const int N = bk_params->tlwe_params->N;
        LweBootstrappingKey* bk = new_LweBootstrappingKey(ks_t, ks_basebit, in_out_params, bk_params);
        random_ks_key(bk->ks);
        double variance = rand()/double(RAND_MAX);
        for (int i=0; i<n; i++) {
            for (int p=0; p<kpl; p++) {
                TLweSample& sample = bk->bk[i].all_sample[p];
                for (int j=0; j<=k; j++) {
                    for (int x=0; x<N; x++)
                        sample.a[j].coefsT[x]=rand();
                }
                sample.current_variance=variance;
            }
        }
        return bk;
    }

    const LweKey* lwekey500 = new_random_lwe_key(lweparams500);
    const set<const LweKey*> allkey = { lwekey500 };

    const TLweKey* tlwekey1024_1 = new_random_tlwe_key(tlweparams1024_1);
    const set<const TLweKey*> allkey_tlwe = { tlwekey1024_1 };

    const TGswKey* tgswkey1024_1 = new_random_tgsw_key(tgswparams1024_1);
    const set<const TGswKey*> allkey_tgsw = { tgswkey1024_1 };
    
    const LweKeySwitchKey* ks503 = new_random_ks_key(503,7,2,lweparams500);
    const set<const LweKeySwitchKey*> allks = { ks503 };

    const LweBootstrappingKey* bk0 = new_random_bk_key(11,1,lweparams500, tgswparams1024_1);
    const set<const LweBootstrappingKey*> allbk = { bk0 };


    //equality test for parameters
    void assert_equals(const LweParams* a, const LweParams* b) {
	ASSERT_EQ(a->n, b->n);	
	ASSERT_DOUBLE_EQ(a->alpha_min, b->alpha_min);	
	ASSERT_DOUBLE_EQ(a->alpha_max, b->alpha_max);
    }

    //equality test for parameters
    void assert_equals(const TLweParams* a, const TLweParams* b) {
	ASSERT_EQ(a->N, b->N);  
	ASSERT_EQ(a->k, b->k);  
	ASSERT_DOUBLE_EQ(a->alpha_min, b->alpha_min);   
	ASSERT_DOUBLE_EQ(a->alpha_max, b->alpha_max);
    }

    //equality test for parameters
    void assert_equals(const TGswParams* a, const TGswParams* b) {
	ASSERT_EQ(a->l, b->l);  
	ASSERT_EQ(a->Bgbit, b->Bgbit);  
	assert_equals(a->tlwe_params, b->tlwe_params);
    }	

    //equality test for keys
    void assert_equals(const LweKey* a, const LweKey* b) {
	assert_equals(a->params,b->params);
	const int n = a->params->n;
	for (int i=0; i<n; i++)
	    ASSERT_EQ(a->key[i], b->key[i]);	
    }

    //equality test for keys
    void assert_equals(const TLweKey* a, const TLweKey* b) {
	assert_equals(a->params,b->params);
	const int N = a->params->N;
	const int k = a->params->k;
	for (int i=0; i<k; i++)
	    for (int j=0; j<N; j++)
		ASSERT_EQ(a->key[i].coefs[j], b->key[i].coefs[j]);	
    }

    //equality test for keys
    void assert_equals(const TGswKey* a, const TGswKey* b) {
	assert_equals(a->params,b->params);
	assert_equals(&a->tlwe_key,&b->tlwe_key);
    }

    //generate a random sample
    void lweSampleUniform(LweSample* a, const LweParams* params) {
	const int n = params->n;
	for (int i=0; i<n; i++) a->a[i]=rand();
	a->b=rand();
	a->current_variance=rand()/double(RAND_MAX);
    }

    //generate a random sample
    void tlweSampleUniform(TLweSample* a, const TLweParams* params) {
	const int k = params->k;
	for (int i=0; i<=k; i++) 
	    torusPolynomialUniform(a->a+i);
	a->current_variance=rand()/double(RAND_MAX);
    }

    //generate a random sample
    void tgswSampleUniform(TGswSample* a, const TGswParams* params) {
	const int kpl = params->kpl;
	const TLweParams* tlwe_params = params->tlwe_params;
	for (int i=0; i<kpl; i++) 
	    tlweSampleUniform(a->all_sample+i,tlwe_params);
    }

    //equality test for samples
    void assert_equals(const LweSample* a, const LweSample* b, const LweParams* params) {
	const int n = params->n;
	for (int i=0; i<n; i++) ASSERT_EQ(a->a[i], b->a[i]);
	ASSERT_EQ(a->b,b->b);
	ASSERT_DOUBLE_EQ(a->current_variance,b->current_variance);	
    }

    //equality test for samples
    void assert_equals(const TLweSample* a, const TLweSample* b, const TLweParams* params) {
	const int k = params->k;
	for (int i=0; i<=k; i++) 
	    ASSERT_EQ(torusPolynomialNormInftyDist(a->a+i, b->a+i),0);
	ASSERT_DOUBLE_EQ(a->current_variance,b->current_variance);	
    }

    //equality test for samples
    void assert_equals(const TGswSample* a, const TGswSample* b, const TGswParams* params) {
	const int kpl = params->kpl;
	const TLweParams* tlwe_params = params->tlwe_params;
	for (int i=0; i<kpl; i++) 
	    assert_equals(a->all_sample+i,b->all_sample+i,tlwe_params);
    }

    //equality test for keyswitch key (for the variance, only the max is
    //compared)
    void assert_equals(const LweKeySwitchKey* a, const LweKeySwitchKey* b) {
	ASSERT_EQ(a->n,b->n);
	ASSERT_EQ(a->t,b->t);
	ASSERT_EQ(a->basebit,b->basebit);
	ASSERT_EQ(a->base,b->base);
	assert_equals(a->out_params, b->out_params);
	const int length = a->n * a->t * a->base;
	const int outn = a->out_params->n;
	double max_vara=-1;
	double max_varb=-1;
	for (int i=0; i<length; i++) {
	    const LweSample& sa = a->ks0_raw[i];
	    const LweSample& sb = b->ks0_raw[i];
	    for (int j=0; j<outn; j++) ASSERT_EQ(sa.a[j],sb.a[j]);
	    ASSERT_EQ(sa.b,sb.b);
	    if (sa.current_variance>max_vara) max_vara=sa.current_variance;
	    if (sb.current_variance>max_varb) max_varb=sb.current_variance;
	}
	ASSERT_EQ(max_vara,max_varb);
    }

    //equality test for bootstrapping key
    void assert_equals(const LweBootstrappingKey* a, const LweBootstrappingKey* b) {
        const int n = a->in_out_params->n;
        const int kpl = a->bk_params->kpl;
        //const int N = a->bk_params->tlwe_params->N;
        const int k = a->bk_params->tlwe_params->k;
        //compare ks
        assert_equals(a->ks, b->ks);
        //compute the max variance
        double max_vara = -1;
        double max_varb = -1;
        for (int i=0; i<n; i++)
            for (int j=0; j<kpl; j++) {
                TLweSample& samplea = a->bk[i].all_sample[j];
                TLweSample& sampleb = b->bk[i].all_sample[j];
                if (samplea.current_variance > max_vara)
                    max_vara = samplea.current_variance;
                if (sampleb.current_variance > max_varb)
                    max_varb = sampleb.current_variance;
            }
        ASSERT_EQ(max_vara,max_varb);
        //compare the coefficients
        for (int i=0; i<n; i++)
            for (int j=0; j<kpl; j++) {
                TLweSample& samplea = a->bk[i].all_sample[j];
                TLweSample& sampleb = b->bk[i].all_sample[j];
                for (int l=0; l<=k; l++)
                    ASSERT_EQ(torusPolynomialNormInftyDist(samplea.a+l,sampleb.a+l),0);
            }
    }

    //equality test for gb parameters
    void assert_equals(const TFheGateBootstrappingParameterSet* a, const TFheGateBootstrappingParameterSet* b) {
        ASSERT_EQ(a->ks_t,b->ks_t);
        ASSERT_EQ(a->ks_basebit,b->ks_basebit);
        assert_equals(a->in_out_params, b->in_out_params);
        assert_equals(a->tgsw_params, b->tgsw_params);
    }

    TEST(IOTest, LweParamsIO) {
	for (const LweParams* params: allparams) {
	    ostringstream oss;
	    export_lweParams_toFile(stdout, params);
	    export_lweParams_toStream(oss, params);
	    string result = oss.str();
	    istringstream iss(result);
	    LweParams* blah = new_lweParams_fromStream(iss);
	    assert_equals(params, blah);
	    delete_LweParams(blah);
	}	
    }



    TEST(IOTest, TLweParamsIO) {
	for (const TLweParams* params: allparams_tlwe) {
	    ostringstream oss;
	    export_tLweParams_toFile(stdout, params);
	    export_tLweParams_toStream(oss, params);
	    string result = oss.str();
	    istringstream iss(result);
	    TLweParams* blah = new_tLweParams_fromStream(iss);
	    assert_equals(params, blah);
	    delete_TLweParams(blah);
	}   
    }



    TEST(IOTest, TGswParamsIO) {
	for (const TGswParams* params: allparams_tgsw) {
	    ostringstream oss;
	    export_tGswParams_toFile(stdout, params);
	    export_tGswParams_toStream(oss, params);
	    string result = oss.str();
	    istringstream iss(result);
	    TGswParams* blah = new_tGswParams_fromStream(iss);
	    assert_equals(params, blah);
	    delete_TGswParams(blah);
	}   
    }


    TEST(IOTest, LweKeyIO) {
	for (const LweKey* key: allkey) {
	    ostringstream oss;
	    //export_lweKey_toFile(stdout, key);
	    export_lweKey_toStream(oss, key);
	    string result = oss.str();
	    istringstream iss(result);
	    LweKey* blah = new_lweKey_fromStream(iss);
	    assert_equals(key, blah);
	    delete_LweKey(blah);
	}	
    }



    TEST(IOTest, TLweKeyIO) {
	for (const TLweKey* key: allkey_tlwe) {
	    ostringstream oss;
	    //export_tLweKey_toFile(stdout, key);
	    export_tlweKey_toStream(oss, key);
	    string result = oss.str();
	    istringstream iss(result);
	    TLweKey* blah = new_tlweKey_fromStream(iss);
	    assert_equals(key, blah);
	    delete_TLweKey(blah);
	}   
    }



    TEST(IOTest, TGswKeyIO) {
	for (const TGswKey* key: allkey_tgsw) {
	    ostringstream oss;
	    //export_tGswKey_toFile(stdout, key);
	    export_tgswKey_toStream(oss, key);
	    string result = oss.str();
	    istringstream iss(result);
	    TGswKey* blah = new_tgswKey_fromStream(iss);
	    assert_equals(key, blah);
	    delete_TGswKey(blah);
	}   
    }


    TEST(IOTest, LweSampleIO) {
	for (const LweParams* params: allparams) {
	    LweSample* sample = new_LweSample(params);
	    LweSample* blah = new_LweSample(params);
	    lweSampleUniform(sample, params);
	    lweSampleUniform(blah, params);
	    ostringstream oss;
	    //export_lweSample_toFile(stdout, sample);
	    export_lweSample_toStream(oss, sample, params);
	    string result = oss.str();
	    istringstream iss(result);
	    import_lweSample_fromStream(iss, blah, params);
	    assert_equals(sample, blah, params);
	    delete_LweSample(blah);
	    delete_LweSample(sample);
	}	
    }


    TEST(IOTest, TLweSampleIO) {
	for (const TLweParams* params: allparams_tlwe) {
	    TLweSample* sample = new_TLweSample(params);
	    TLweSample* blah = new_TLweSample(params);
	    tlweSampleUniform(sample, params);
	    tlweSampleUniform(blah, params);
	    ostringstream oss;
	    //export_lweSample_toFile(stdout, sample);
	    export_tlweSample_toStream(oss, sample, params);
	    string result = oss.str();
	    istringstream iss(result);
	    import_tlweSample_fromStream(iss, blah, params);
	    assert_equals(sample, blah, params);
	    delete_TLweSample(blah);
	    delete_TLweSample(sample);
	}	
    }

    TEST(IOTest, TGswSampleIO) {
        for (const TGswParams* params: allparams_tgsw) {
            TGswSample* sample = new_TGswSample(params);
            TGswSample* blah = new_TGswSample(params);
            tgswSampleUniform(sample, params);
            tgswSampleUniform(blah, params);
            ostringstream oss;
            //export_gswSample_toFile(stdout, sample);
            export_tgswSample_toStream(oss, sample, params);
            string result = oss.str();
            istringstream iss(result);
            import_tgswSample_fromStream(iss, blah, params);
            assert_equals(sample, blah, params);
            delete_TGswSample(blah);
            delete_TGswSample(sample);
        }
    }


    TEST(IOTest, LweKeySwitchKeyIO) {
        for (const LweKeySwitchKey* ks: allks) {
            {
                ostringstream oss;
                export_lweKeySwitchKey_toStream(oss, ks);
                string result = oss.str();
                istringstream iss(result);
                LweKeySwitchKey* ks1 = new_lweKeySwitchKey_fromStream(iss);
                assert_equals(ks,ks1);
                delete_LweKeySwitchKey(ks1);
            }
        }	
    }

    TEST(IOTest, LweBootstrappingKeyIO) {
        for (const LweBootstrappingKey* bk: allbk) {
            {
                ostringstream oss;
                export_lweBootstrappingKey_toStream(oss, bk);
                string result = oss.str();
                istringstream iss(result);
                LweBootstrappingKey* bk1 = new_lweBootstrappingKey_fromStream(iss);
                assert_equals(bk,bk1);
                delete_LweBootstrappingKey(bk1);
            }
        }	
    }

    TEST(IOTest, TFheGateBootstrappingParameterSetIO) {
        for (const TFheGateBootstrappingParameterSet* gbp: allgbp) {
            {
                ostringstream oss;
                export_tfheGateBootstrappingParameterSet_toStream(oss, gbp);
                export_tfheGateBootstrappingParameterSet_toFile(stdout, gbp);
                string result = oss.str();
                istringstream iss(result);
                TFheGateBootstrappingParameterSet* gbp1 = new_tfheGateBootstrappingParameterSet_fromStream(iss);
                assert_equals(gbp,gbp1);
                delete_gate_bootstrapping_parameters(gbp1);
            }
        }	
    }

}
