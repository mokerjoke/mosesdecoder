// Query binary phrase tables.
// Marcin Junczys-Dowmunt, 13 September 2012

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "CompactPT/PhraseDictionaryCompact.h"
#include "Util.h"

void usage();

typedef unsigned int uint;

using namespace Moses;

int main(int argc, char **argv)
{
  int nscores = 5;
  std::string ttable = "";
  bool useAlignments = false;
  bool reportCounts = false;

  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-n")) {
      if(i + 1 == argc)
        usage();
      nscores = atoi(argv[++i]);
    } else if(!strcmp(argv[i], "-t")) {
      if(i + 1 == argc)
        usage();
      ttable = argv[++i];
    } else if(!strcmp(argv[i], "-a")) {
      useAlignments = true;
    } else if (!strcmp(argv[i], "-c")) {
      reportCounts = true;
    }
    else
      usage();
  }

  if(ttable == "")
    usage();

  std::vector<FactorType> input(1, 0);
  std::vector<FactorType> output(1, 0);
  std::vector<float> weight(nscores, 0);
  
  LMList lmList;
  
  PhraseDictionaryFeature pdf(Compact, nscores, nscores, input, output, ttable, weight, 0, "", "");
  PhraseDictionaryCompact pdc(nscores, Compact, &pdf, false, useAlignments);
  bool ret = pdc.Load(input, output, ttable, weight, 0, lmList, 0);                                                                           
  assert(ret);
  
  std::string line;
  while(getline(std::cin, line)) {
    Phrase sourcePhrase(0);
    sourcePhrase.CreateFromString(input, line, "||dummy_string||");
    
    TargetPhraseVectorPtr decodedPhraseColl
      = pdc.GetTargetPhraseCollectionRaw(sourcePhrase);
    
    if(decodedPhraseColl != NULL) {
      if(reportCounts)
        std::cout << sourcePhrase << decodedPhraseColl->size() << std::endl;
      else
        for(TargetPhraseVector::iterator it = decodedPhraseColl->begin(); it != decodedPhraseColl->end(); it++) {
          TargetPhrase &tp = *it;
          std::cout << sourcePhrase << "||| ";
          std::cout << static_cast<const Phrase&>(tp) << "|||";
          
          if(useAlignments)
            std::cout << " " << tp.GetAlignmentInfo() << "|||"; 
          
          for(size_t i = 0; i < tp.GetScoreBreakdown().size(); i++)
            std::cout << " " << exp(tp.GetScoreBreakdown()[i]);
          std::cout << std::endl;
        }
    }
    else if(reportCounts)
      std::cout << sourcePhrase << 0 << std::endl;
    
    std::cout.flush();
  }
}

void usage()
{
  std::cerr << 	"Usage: queryPhraseTable [-n <nscores>] [-a] -t <ttable>\n"
            "-n <nscores>      number of scores in phrase table (default: 5)\n"
            "-c                only report counts of entries\n"
            "-a                binary phrase table contains alignments\n"
            "-t <ttable>       phrase table\n";
  exit(1);
}
