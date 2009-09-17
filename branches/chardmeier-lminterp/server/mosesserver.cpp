#include <cassert>
#include <stdexcept>
#include <iostream>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include "Hypothesis.h"
#include "Manager.h"
#include "StaticData.h"


using namespace Moses;
using namespace std;


class Translator : public xmlrpc_c::method {
public:
    Translator() {
        // signature and help strings are documentation -- the client
        // can query this information with a system.methodSignature and
        // system.methodHelp RPC.
        this->_signature = "S:S";
        this->_help = "Does translation";
    }

    typedef std::map<std::string, xmlrpc_c::value> params_t;

    void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value *   const  retvalP) {
        
        const params_t params = paramList.getStruct(0);
        paramList.verifyEnd(1);
        params_t::const_iterator si = params.find("text");
        if (si == params.end()) {
            throw xmlrpc_c::fault(
                "Missing source text", 
                xmlrpc_c::fault::CODE_PARSE);
        }
        const string source(
            (xmlrpc_c::value_string(si->second)));

        cerr << "Input: " << source << endl;
        si = params.find("align");
        bool addAlignInfo = (si != params.end());

        const StaticData &staticData = StaticData::Instance();
        Sentence sentence(Input);
        const vector<FactorType> &inputFactorOrder = 
            staticData.GetInputFactorOrder();
        stringstream in(source + "\n");
        sentence.Read(in,inputFactorOrder);
        Manager manager(sentence,staticData.GetSearchAlgorithm());
        manager.ProcessSentence();
        const Hypothesis* hypo = manager.GetBestHypothesis();

        vector<xmlrpc_c::value> alignInfo;
        stringstream out;
        outputHypo(out,hypo,addAlignInfo,alignInfo);

        map<string, xmlrpc_c::value> retData;
        pair<string, xmlrpc_c::value> 
            text("text", xmlrpc_c::value_string(out.str()));
        cerr << "Output: " << out.str() << endl;
        if (addAlignInfo) {
            retData.insert(pair<string, xmlrpc_c::value>("align", xmlrpc_c::value_array(alignInfo)));
        }
        retData.insert(text);
        
        *retvalP = xmlrpc_c::value_struct(retData);

    }

    void outputHypo(ostream& out, const Hypothesis* hypo, bool addAlignmentInfo, vector<xmlrpc_c::value>& alignInfo) {
        if (hypo->GetPrevHypo() != NULL) {
            outputHypo(out,hypo->GetPrevHypo(),addAlignmentInfo, alignInfo);
            TargetPhrase p = hypo->GetTargetPhrase();
            for (size_t pos = 0 ; pos < p.GetSize() ; pos++)
            {
                const Factor *factor = p.GetFactor(pos, 0);
                out << *factor << " ";

            }
            if (addAlignmentInfo) {
            /**
             * Add the alignment info to the array. This is in target order and consists of 
             *       (tgt-start, src-start, src-end) triples. 
             **/
                map<string, xmlrpc_c::value> phraseAlignInfo;
                phraseAlignInfo["tgt-start"] = xmlrpc_c::value_int(hypo->GetCurrTargetWordsRange().GetStartPos());
                phraseAlignInfo["src-start"] = xmlrpc_c::value_int(hypo->GetCurrSourceWordsRange().GetStartPos());
                phraseAlignInfo["src-end"] = xmlrpc_c::value_int(hypo->GetCurrSourceWordsRange().GetEndPos());
                alignInfo.push_back(xmlrpc_c::value_struct(phraseAlignInfo));
            }
        }
    }
    
    
    
};


int main(int argc, char** argv) {

    //Extract port and log, send other args to moses
    char** mosesargv = new char*[argc+2];
    int mosesargc = 0;
    int port = 8080;
    const char* logfile = "/dev/null";

    for (int i = 0; i < argc; ++i) {
        if (!strcmp(argv[i],"--server-port")) {
            ++i;
            if (i >= argc) {
                cerr << "Error: Missing argument to --server-port" << endl;
                exit(1);
            } else {
                port = atoi(argv[i]);
            }
        } else if (!strcmp(argv[i],"--server-log")) {
            ++i;
            if (i >= argc) {
                cerr << "Error: Missing argument to --server-log" << endl;
                exit(1);
            } else {
                logfile = argv[i];
            }
        } else {
            mosesargv[mosesargc] = new char[strlen(argv[i])+1];
            strcpy(mosesargv[mosesargc],argv[i]);
            ++mosesargc;
        }
    }

    Parameter* params = new Parameter();
    if (!params->LoadParam(mosesargc,mosesargv)) {
        params->Explain();
        exit(1);
    }
    if (!StaticData::LoadDataStatic(params)) {
        exit(1);
    }

    xmlrpc_c::registry myRegistry;

    xmlrpc_c::methodPtr const translator(new Translator);

    myRegistry.addMethod("translate", translator);
    
    xmlrpc_c::serverAbyss myAbyssServer(
        myRegistry,
        port,              // TCP port on which to listen
        logfile  
        );
    
    cerr << "Listening on port " << port << endl;
    myAbyssServer.run();
    // xmlrpc_c::serverAbyss.run() never returns
    assert(false);
    return 0;
}
