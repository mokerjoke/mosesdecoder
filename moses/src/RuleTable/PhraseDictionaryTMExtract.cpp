// vim:tabstop=2

/***********************************************************************
 Moses - factored phrase-based language decoder
 Copyright (C) 2006 University of Edinburgh
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***********************************************************************/

#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include "RuleTable/Loader.h"
#include "RuleTable/LoaderFactory.h"
#include "PhraseDictionaryTMExtract.h"
#include "FactorCollection.h"
#include "Word.h"
#include "Util.h"
#include "InputFileStream.h"
#include "StaticData.h"
#include "WordsRange.h"
#include "UserMessage.h"
#include "CYKPlusParser/ChartRuleLookupManagerMemoryPerSentence.h"

using namespace std;

namespace Moses
{
  PhraseDictionaryTMExtract::PhraseDictionaryTMExtract(size_t numScoreComponents,
                            PhraseDictionaryFeature* feature)
  : PhraseDictionary(numScoreComponents, feature) 
  {
    const StaticData &staticData = StaticData::Instance();
    CHECK(staticData.ThreadCount() == 1);
  }

  TargetPhraseCollection &PhraseDictionaryTMExtract::GetOrCreateTargetPhraseCollection(
                                                                                  const Phrase &source
                                                                                  , const TargetPhrase &target
                                                                                  , const Word &sourceLHS)
  {
    PhraseDictionaryNodeSCFG &currNode = GetOrCreateNode(source, target, sourceLHS);
    return currNode.GetOrCreateTargetPhraseCollection();
  }
  
  PhraseDictionaryNodeSCFG &PhraseDictionaryTMExtract::GetOrCreateNode(const Phrase &source
                                                                  , const TargetPhrase &target
                                                                  , const Word &sourceLHS)
  {
    const size_t size = source.GetSize();
    
    const AlignmentInfo &alignmentInfo = target.GetAlignmentInfo();
    AlignmentInfo::const_iterator iterAlign = alignmentInfo.begin();
    
    PhraseDictionaryNodeSCFG *currNode = &m_collection;
    for (size_t pos = 0 ; pos < size ; ++pos) {
      const Word& word = source.GetWord(pos);
      
      if (word.IsNonTerminal()) {
        // indexed by source label 1st
        const Word &sourceNonTerm = word;
        
        CHECK(iterAlign != target.GetAlignmentInfo().end());
        CHECK(iterAlign->first == pos);
        size_t targetNonTermInd = iterAlign->second;
        ++iterAlign;
        const Word &targetNonTerm = target.GetWord(targetNonTermInd);
        
        currNode = currNode->GetOrCreateChild(sourceNonTerm, targetNonTerm);
      } else {
        currNode = currNode->GetOrCreateChild(word);
      }
      
      CHECK(currNode != NULL);
    }
    
    // finally, the source LHS
    //currNode = currNode->GetOrCreateChild(sourceLHS);
    //CHECK(currNode != NULL);
    
    
    return *currNode;
  }
  
  ChartRuleLookupManager *PhraseDictionaryTMExtract::CreateRuleLookupManager(
                                                                        const InputType &sentence,
                                                                        const ChartCellCollection &cellCollection)
  {
    return new ChartRuleLookupManagerMemoryPerSentence(sentence, cellCollection, *this);
  }
  
  void PhraseDictionaryTMExtract::SortAndPrune()
  {
    if (GetTableLimit())
    {
      m_collection.Sort(GetTableLimit());
    }
  }
  
  void PhraseDictionaryTMExtract::InitializeForInput(InputType const& source)
  {
    /*
    string data_root = "/Users/hieuhoang/workspace/experiment/data/tm-mt-integration/";
    string pt_file = data_root + "out/pt";
    string cmd = "perl ~/workspace/github/hieuhoang/contrib/tm-mt-integration/make-pt-from-tm.perl "
    + data_root + "in/ac-test.input.tc.4 "
    + data_root + "in/acquis.truecased.4.en.uniq "
    + data_root + "in/acquis.truecased.4.fr.uniq "
    + data_root + "in/acquis.truecased.4.align.uniq "
    + data_root + "in/lex.4 "
    + pt_file;
    system(cmd.c_str());
     */
    
    
  }
  
  void PhraseDictionaryTMExtract::CleanUp(const InputType &source)
  {
    m_collection.Clear();
  }


  TO_STRING_BODY(PhraseDictionaryTMExtract);
  
  // friend
  ostream& operator<<(ostream& out, const PhraseDictionaryTMExtract& phraseDict)
  {
    typedef PhraseDictionaryNodeSCFG::TerminalMap TermMap;
    typedef PhraseDictionaryNodeSCFG::NonTerminalMap NonTermMap;
    
    const PhraseDictionaryNodeSCFG &coll = phraseDict.m_collection;
    for (NonTermMap::const_iterator p = coll.m_nonTermMap.begin(); p != coll.m_nonTermMap.end(); ++p) {
      const Word &sourceNonTerm = p->first.first;
      out << sourceNonTerm;
    }
    for (TermMap::const_iterator p = coll.m_sourceTermMap.begin(); p != coll.m_sourceTermMap.end(); ++p) {
      const Word &sourceTerm = p->first;
      out << sourceTerm;
    }

    return out;
  }
  
}