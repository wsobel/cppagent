//
// Copyright Copyright 2009-2019, AMT – The Association For Manufacturing Technology (“AMT”)
// All rights reserved.
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//

#include "Cuti.h"
#include <stdio.h>
#include "adapter.hpp"
#include "agent.hpp"
#include "test_globals.hpp"
#include "agent_test_helper.hpp"


using namespace std;
using namespace mtconnect;

TEST_CLASS(DataSetTest)
{
  
protected:
  std::unique_ptr<Checkpoint> m_checkpoint;
  std::unique_ptr<Agent> m_agent;
  Adapter *m_adapter;
  std::string m_agentId;
  DataItem* m_dataItem1;
  
  std::unique_ptr<AgentTestHelper> m_agentTestHelper;
  
public:
  void testDataItem();
  void testInitialSet();
  void testUpdateOneElement();
  void testUpdateMany();
  void testReset();
  void testBadData();
  void testCurrent();
  void testSample();
  void testCurrentAt();
  void testDeleteKey();
  void testResetWithNoItems();
  void testDuplicateCompression();
  void testQuoteDelimeter();
  void testSampleWithDiscrete();
  void testProbe();

  SET_UP();
  TEAR_DOWN();
  
  CPPUNIT_TEST_SUITE(DataSetTest);
  
  CPPUNIT_TEST(testDataItem);
  CPPUNIT_TEST(testInitialSet);
  CPPUNIT_TEST(testUpdateOneElement);
  CPPUNIT_TEST(testUpdateMany);
  CPPUNIT_TEST(testReset);
  CPPUNIT_TEST(testBadData);
  CPPUNIT_TEST(testCurrent);
  CPPUNIT_TEST(testSample);
  CPPUNIT_TEST(testCurrentAt);
  CPPUNIT_TEST(testDeleteKey);
  CPPUNIT_TEST(testResetWithNoItems);
  CPPUNIT_TEST(testDuplicateCompression);
  CPPUNIT_TEST(testQuoteDelimeter);
  CPPUNIT_TEST(testSampleWithDiscrete);
  CPPUNIT_TEST(testProbe);
  
  CPPUNIT_TEST_SUITE_END();

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(DataSetTest);

inline DataSetEntry operator "" _E(const char *c, std::size_t) {
  return DataSetEntry(c);
}

void DataSetTest::setUp()
{
  // Create an agent with only 16 slots and 8 data items.
  m_checkpoint = nullptr;
  m_agent = make_unique<Agent>(PROJECT_ROOT_DIR "/samples/data_set.xml", 4, 4, "1.5");
  m_agentId = int64ToString(getCurrentTimeInSec());
  m_adapter = nullptr;
  m_checkpoint = make_unique<Checkpoint>();
  
  m_agentTestHelper = make_unique<AgentTestHelper>();
  m_agentTestHelper->m_agent = m_agent.get();
#ifdef __MACH__
  m_agentTestHelper->self = self;
#endif
  
  std::map<string, string> attributes;
  auto device = m_agent->getDeviceByName("LinuxCNC");
  m_dataItem1 = device->getDeviceDataItem("v1");
}


void DataSetTest::tearDown()
{
  m_agent.reset();
  m_checkpoint.reset();
  m_agentTestHelper.reset();
}

void DataSetTest::testDataItem()
{
  CPPUNIT_ASSERT(m_dataItem1->isDataSet());
  auto &attrs = m_dataItem1->getAttributes();
  
  CPPUNIT_ASSERT_EQUAL((string) "DATA_SET", attrs.at("representation"));
  CPPUNIT_ASSERT_EQUAL((string) "VariableDataSet", m_dataItem1->getElementName());
}

void DataSetTest::testInitialSet()
{
  string value("a=1 b=2 c=3 d=4");
  auto ce = new Observation(*m_dataItem1, 2, "time", value);
  
  CPPUNIT_ASSERT_EQUAL((size_t) 4, ce->getDataSet().size());
  auto &al = ce->getAttributes();
  std::map<string, string> attrs;
  
  for (const auto &attr : al)
    attrs[attr.first] = attr.second;
  
  CPPUNIT_ASSERT_EQUAL((string) "4", attrs.at("count"));
  
  auto map1 = ce->getDataSet();
  
  CPPUNIT_ASSERT_EQUAL((string) "1", map1.find("a"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "2", map1.find("b"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "3", map1.find("c"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "4", map1.find("d"_E)->m_value);
  
  m_checkpoint->addObservation(ce);
  auto c2 = *m_checkpoint->getEventPtr("v1");
  auto al2 = c2->getAttributes();
  
  attrs.clear();
  for (const auto &attr : al2)
    attrs[attr.first] = attr.second;
  
  CPPUNIT_ASSERT_EQUAL((string) "4", attrs.at("count"));
  
  auto map2 = c2->getDataSet();
  CPPUNIT_ASSERT_EQUAL((string) "1", map2.find("a"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "2", map2.find("b"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "3", map2.find("c"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "4", map2.find("d"_E)->m_value);
  
  ce->unrefer();
}

void DataSetTest::testUpdateOneElement()
{
  string value("a=1 b=2 c=3 d=4");
  ObservationPtr ce(new Observation(*m_dataItem1, 2, "time", value));
  m_checkpoint->addObservation(ce);
  
  auto cecp = *m_checkpoint->getEventPtr("v1");
  CPPUNIT_ASSERT_EQUAL((size_t) 4, cecp->getDataSet().size());
  
  string value2("c=5");
  ObservationPtr ce2(new Observation(*m_dataItem1, 2, "time", value2));
  m_checkpoint->addObservation(ce2);
  
  auto ce3 = *m_checkpoint->getEventPtr("v1");
  CPPUNIT_ASSERT_EQUAL((size_t) 4, ce3->getDataSet().size());
  
  auto map1 = ce3->getDataSet();
  CPPUNIT_ASSERT_EQUAL((string) "1", map1.find("a"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "2", map1.find("b"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "5", map1.find("c"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "4", map1.find("d"_E)->m_value);
  
  string value3("e=6");
  ObservationPtr ce4(new Observation(*m_dataItem1, 2, "time", value3));
  m_checkpoint->addObservation(ce4);
  
  auto ce5 = *m_checkpoint->getEventPtr("v1");
  CPPUNIT_ASSERT_EQUAL((size_t) 5, ce5->getDataSet().size());
  
  auto map2 = ce5->getDataSet();
  CPPUNIT_ASSERT_EQUAL((string) "1", map2.find("a"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "2", map2.find("b"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "5", map2.find("c"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "4", map2.find("d"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "6", map2.find("e"_E)->m_value);
}

void DataSetTest::testUpdateMany()
{
  string value("a=1 b=2 c=3 d=4");
  ObservationPtr ce(new Observation(*m_dataItem1, 2, "time", value));
  m_checkpoint->addObservation(ce);
  
  auto cecp = *m_checkpoint->getEventPtr("v1");
  CPPUNIT_ASSERT_EQUAL((size_t) 4, cecp->getDataSet().size());
  
  string value2("c=5 e=6");
  ObservationPtr ce2(new Observation(*m_dataItem1, 2, "time", value2));
  m_checkpoint->addObservation(ce2);
  
  auto ce3 = *m_checkpoint->getEventPtr("v1");
  CPPUNIT_ASSERT_EQUAL((size_t) 5, ce3->getDataSet().size());
  
  auto map1 = ce3->getDataSet();
  CPPUNIT_ASSERT_EQUAL((string) "1", map1.find("a"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "2", map1.find("b"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "5", map1.find("c"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "4", map1.find("d"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "6", map1.find("e"_E)->m_value);
  
  string value3("e=7 a=8 f=9");
  ObservationPtr ce4(new Observation(*m_dataItem1, 2, "time", value3));
  m_checkpoint->addObservation(ce4);
  
  auto ce5 = *m_checkpoint->getEventPtr("v1");
  CPPUNIT_ASSERT_EQUAL((size_t) 6, ce5->getDataSet().size());
  
  auto map2 = ce5->getDataSet();
  CPPUNIT_ASSERT_EQUAL((string) "8", map2.find("a"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "2", map2.find("b"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "5", map2.find("c"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "4", map2.find("d"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "7", map2.find("e"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "9", map2.find("f"_E)->m_value);
}

void DataSetTest::testReset()
{
  string value("a=1 b=2 c=3 d=4");
  ObservationPtr ce(new Observation(*m_dataItem1, 2, "time", value));
  m_checkpoint->addObservation(ce);
  
  auto cecp = *m_checkpoint->getEventPtr("v1");
  CPPUNIT_ASSERT_EQUAL((size_t) 4, cecp->getDataSet().size());
  
  string value2(":MANUAL c=5 e=6");
  ObservationPtr ce2(new Observation(*m_dataItem1, 2, "time", value2));
  m_checkpoint->addObservation(ce2);
  
  auto ce3 = *m_checkpoint->getEventPtr("v1");
  CPPUNIT_ASSERT_EQUAL((size_t) 2, ce3->getDataSet().size());
  
  auto map1 = ce3->getDataSet();
  CPPUNIT_ASSERT_EQUAL((string) "5", map1.find("c"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "6", map1.find("e"_E)->m_value);
  
  string value3("x=pop y=hop");
  ObservationPtr ce4(new Observation(*m_dataItem1, 2, "time", value3));
  m_checkpoint->addObservation(ce4);
  
  auto ce5 = *m_checkpoint->getEventPtr("v1");
  CPPUNIT_ASSERT_EQUAL((size_t) 4, ce5->getDataSet().size());
  
  auto map2 = ce5->getDataSet();
  CPPUNIT_ASSERT_EQUAL((string) "pop", map2.find("x"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "hop", map2.find("y"_E)->m_value);
}

void DataSetTest::testBadData()
{
  string value("12356");
  auto ce = new Observation(*m_dataItem1, 2, "time", value);
  
  CPPUNIT_ASSERT_EQUAL((size_t) 1, ce->getDataSet().size());
  
  string value1("  a=2      b3=xxx");
  auto ce2 = new Observation(*m_dataItem1, 2, "time", value1);
  
  CPPUNIT_ASSERT_EQUAL((size_t) 2, ce2->getDataSet().size());
  
  auto map1 = ce2->getDataSet();
  CPPUNIT_ASSERT_EQUAL((string) "2", map1.find("a"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "xxx", map1.find("b3"_E)->m_value);
}

#define ASSETT_DATA_SET_ENTRY(doc, var, key, expected) \
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:" \
               var "/m:Entry[@key='" key "']", expected)

void DataSetTest::testCurrent()
{
  m_adapter = m_agent->addAdapter("LinuxCNC", "server", 7878, false);
  CPPUNIT_ASSERT(m_adapter);
  
  m_agentTestHelper->m_path = "/current";
  
  {
    PARSE_XML_RESPONSE;
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc,
                                      "//m:DeviceStream//m:VariableDataSet[@dataItemId='v1']",
                                      "UNAVAILABLE");
  }
  
  m_adapter->processData("TIME|vars|a=1 b=2 c=3");
  
  {
    PARSE_XML_RESPONSE;
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[@dataItemId='v1']", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[@dataItemId='v1']", "b", "2");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[@dataItemId='v1']", "c", "3");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc,"//m:DeviceStream//m:VariableDataSet"
                                      "[@dataItemId='v1']@count", "3");
  }
  
  m_adapter->processData("TIME|vars|c=6");
  
  {
    PARSE_XML_RESPONSE;
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[@dataItemId='v1']", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[@dataItemId='v1']", "b", "2");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[@dataItemId='v1']", "c", "6");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc,"//m:DeviceStream//m:VariableDataSet"
                                      "[@dataItemId='v1']@count", "3");
  }
  
  m_adapter->processData("TIME|vars|:MANUAL d=10");
  
  {
    PARSE_XML_RESPONSE;
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[@dataItemId='v1']", "d", "10");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc,"//m:DeviceStream//m:VariableDataSet"
                                      "[@dataItemId='v1']@count", "1");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc,
                                      "//m:DeviceStream//m:VariableDataSet"
                                      "[@dataItemId='v1']@resetTriggered",
                                      "MANUAL");
    
  }
  
  m_adapter->processData("TIME|vars|c=6");
  
  {
    PARSE_XML_RESPONSE;
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[@dataItemId='v1']", "c", "6");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[@dataItemId='v1']", "d", "10");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc,"//m:DeviceStream//m:VariableDataSet"
                                      "[@dataItemId='v1']@count", "2");
  }
}

void DataSetTest::testSample()
{
  m_adapter = m_agent->addAdapter("LinuxCNC", "server", 7878, false);
  CPPUNIT_ASSERT(m_adapter);
  
  m_adapter->processData("TIME|vars|a=1 b=2 c=3");
  m_adapter->processData("TIME|vars|c=5");
  m_adapter->processData("TIME|vars|a=1 c=8");
  
  m_agentTestHelper->m_path = "/sample";
  
  {
    PARSE_XML_RESPONSE;
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]", "UNAVAILABLE");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[2]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[2]", "b", "2");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[2]", "c", "3");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[2]@count", "3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[3]", "c", "5");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[3]@count", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[4]", "c", "8");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[4]@count", "1");
  }
  
  m_agentTestHelper->m_path = "/current";
  {
    PARSE_XML_RESPONSE;
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "b", "2");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "c", "8");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@count", "3");
  }
  
  m_agentTestHelper->m_path = "/sample";
  m_adapter->processData("TIME|vars|c b=5");
  
  {
    PARSE_XML_RESPONSE;
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]", "UNAVAILABLE");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[2]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[2]", "b", "2");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[2]", "c", "3");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[2]@count", "3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[3]", "c", "5");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[3]@count", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[4]", "c", "8");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[4]@count", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[5]", "b", "5");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[5]", "c", "");
 
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[4]/m:Entry[@key='c']@removed", nullptr);
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[5]/m:Entry[@key='c']@removed", "true");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[5]@count", "2");
  }
  
  m_agentTestHelper->m_path = "/current";
  
  {
    PARSE_XML_RESPONSE;
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "b", "5");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@count", "2");
  }
  
}

void DataSetTest::testCurrentAt()
{
  m_adapter = m_agent->addAdapter("LinuxCNC", "server", 7878, false);
  CPPUNIT_ASSERT(m_adapter);
  
  auto seq = m_agent->getSequence();
  
  m_adapter->processData("TIME|vars|a=1 b=2 c=3");
  m_adapter->processData("TIME|vars| c=5 ");
  m_adapter->processData("TIME|vars|c=8");
  m_adapter->processData("TIME|vars|b=10   a=xxx");
  m_adapter->processData("TIME|vars|:MANUAL q=hello_there");
  m_adapter->processData("TIME|vars|r=good_bye");
  
  m_agentTestHelper->m_path = "/current";
  
  {
    PARSE_XML_RESPONSE_QUERY_KV("at", int64ToString(seq - 1));
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]", "UNAVAILABLE");
  }
  
  {
    PARSE_XML_RESPONSE_QUERY_KV("at", int64ToString(seq));
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "b", "2");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "c", "3");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@sequence", int64ToString(seq).c_str());
  }
  
  {
    PARSE_XML_RESPONSE_QUERY_KV("at", int64ToString(seq + 1));
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "b", "2");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "c", "5");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@sequence", int64ToString(seq + 1).c_str());
  }
  
  {
    PARSE_XML_RESPONSE_QUERY_KV("at", int64ToString(seq + 2));
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "b", "2");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "c", "8");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@sequence", int64ToString(seq + 2).c_str());
  }
  
  {
    PARSE_XML_RESPONSE_QUERY_KV("at", int64ToString(seq + 3));
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "a", "xxx");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "b", "10");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "c", "8");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@sequence", int64ToString(seq + 3).c_str());
  }
  
  {
    PARSE_XML_RESPONSE_QUERY_KV("at", int64ToString(seq + 4));
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "q", "hello_there");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "///m:VariableDataSet@resetTriggered",
                                      "MANUAL");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@sequence", int64ToString(seq + 4).c_str());
  }
  
  {
    PARSE_XML_RESPONSE_QUERY_KV("at", int64ToString(seq + 5));
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "q", "hello_there");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "r", "good_bye");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@resetTriggered", 0);
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@sequence", int64ToString(seq + 5).c_str());
  }
  
  {
    PARSE_XML_RESPONSE;
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "q", "hello_there");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "r", "good_bye");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@resetTriggered", 0);
  }
}

void DataSetTest::testDeleteKey()
{
  string value("a=1 b=2 c=3 d=4");
  ObservationPtr ce(new Observation(*m_dataItem1, 2, "time", value));
  m_checkpoint->addObservation(ce);
  
  auto cecp = *m_checkpoint->getEventPtr("v1");
  CPPUNIT_ASSERT_EQUAL((size_t) 4, cecp->getDataSet().size());
  
  string value2("c e=6 a");
  ObservationPtr ce2(new Observation(*m_dataItem1, 4, "time", value2));
  m_checkpoint->addObservation(ce2);
  
  
  
  auto ce3 = *m_checkpoint->getEventPtr("v1");
  CPPUNIT_ASSERT_EQUAL((size_t) 3, ce3->getDataSet().size());
  
  auto &ds = ce2->getDataSet();
  CPPUNIT_ASSERT(ds.find("a"_E)->m_removed);

  auto map1 = ce3->getDataSet();
  CPPUNIT_ASSERT_EQUAL((string) "2", map1.find("b"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "4", map1.find("d"_E)->m_value);
  CPPUNIT_ASSERT_EQUAL((string) "6", map1.find("e"_E)->m_value);
  CPPUNIT_ASSERT(map1.find("c"_E) == map1.end());
  CPPUNIT_ASSERT(map1.find("a"_E) == map1.end());
}

void DataSetTest::testResetWithNoItems()
{
  m_adapter = m_agent->addAdapter("LinuxCNC", "server", 7878, false);
  CPPUNIT_ASSERT(m_adapter);
  
  m_adapter->processData("TIME|vars|a=1 b=2 c=3");
  m_adapter->processData("TIME|vars| c=5 ");
  m_adapter->processData("TIME|vars|c=8");
  m_adapter->processData("TIME|vars|b=10   a=xxx");
  m_adapter->processData("TIME|vars|:MANUAL");
  m_adapter->processData("TIME|vars|r=good_bye");
  
  m_agentTestHelper->m_path = "/sample";
  
  {
    PARSE_XML_RESPONSE;
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]", "UNAVAILABLE");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[2]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[2]", "b", "2");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[2]", "c", "3");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[2]@count", "3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[3]", "c", "5");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[3]@count", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[4]", "c", "8");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[4]@count", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[5]", "a", "xxx");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[5]", "b", "10");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[5]@count", "2");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[6]", "");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[6]@count", "0");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[6]@resetTriggered", "MANUAL");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[7]", "r", "good_bye");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[7]@count", "1");
  }
  
}

void DataSetTest::testDuplicateCompression()
{
  m_adapter = m_agent->addAdapter("LinuxCNC", "server", 7878, false);
  CPPUNIT_ASSERT(m_adapter);
  
  m_adapter->processData("TIME|vars|a=1 b=2 c=3");
  m_adapter->processData("TIME|vars|b=2");
  m_adapter->processData("TIME|vars|b=2 d=4");
  m_adapter->processData("TIME|vars|b=2 d=4 c=3");
  m_adapter->processData("TIME|vars|b=2 d=4 c=3");
  m_adapter->processData("TIME|vars|b=3 e=4");
  
  m_agentTestHelper->m_path = "/sample";
  
  {
    PARSE_XML_RESPONSE;
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]", "UNAVAILABLE");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[2]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[2]", "b", "2");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[2]", "c", "3");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[2]@count", "3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[3]", "d", "4");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[3]@count", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[4]", "b", "3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[4]", "e", "4");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[4]@count", "2");
  }
  
  m_agentTestHelper->m_path = "/current";
  
  {
    PARSE_XML_RESPONSE;
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "b", "3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "c", "3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "d", "4");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "e", "4");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@count", "5");
  }
  
  m_adapter->processData("TIME|vars|:MANUAL a=1 b=3 c=3 d=4 e=4");
  
  m_agentTestHelper->m_path = "/sample";
  
  {
    PARSE_XML_RESPONSE;
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[5]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[5]", "b", "3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[5]", "c", "3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[5]", "d", "4");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[5]", "e", "4");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[5]@count", "5");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[5]@resetTriggered", "MANUAL");
    
  }
  
  m_agentTestHelper->m_path = "/current";
  
  {
    PARSE_XML_RESPONSE;
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "b", "3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "c", "3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "d", "4");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "e", "4");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@count", "5");
  }
}

void DataSetTest::testQuoteDelimeter()
{
  m_adapter = m_agent->addAdapter("LinuxCNC", "server", 7878, false);
  CPPUNIT_ASSERT(m_adapter);
  
  m_adapter->processData("TIME|vars|a='1 2 3' b=\"x y z\" c={cats and dogs}");
  
  m_agentTestHelper->m_path = "/current";
  
  {
    PARSE_XML_RESPONSE;
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "a", "1 2 3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "b", "x y z");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "c", "cats and dogs");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@count", "3");
  }
  
  m_adapter->processData("TIME|vars|b='u v w' c={chickens and horses");
  {
    PARSE_XML_RESPONSE;
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "a", "1 2 3");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "b", "u v w");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "c", "cats and dogs");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@count", "3");
  }
  
  m_adapter->processData("TIME|vars|:MANUAL V123={x1.111 2.2222 3.3333} V124={x1.111 2.2222 3.3333} V1754={\"Part 1\" 2.2222 3.3333}");
  {
    PARSE_XML_RESPONSE;
    
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "V123", "x1.111 2.2222 3.3333");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "V124", "x1.111 2.2222 3.3333");
    ASSETT_DATA_SET_ENTRY(doc, "VariableDataSet[1]", "V1754", "\"Part 1\" 2.2222 3.3333");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:VariableDataSet[1]@count", "3");
  }
}

void DataSetTest::testSampleWithDiscrete()
{
  m_adapter = m_agent->addAdapter("LinuxCNC", "server", 7878, false);
  CPPUNIT_ASSERT(m_adapter);
  
  auto di = m_agent->getDataItemByName("LinuxCNC", "vars2");
  CPPUNIT_ASSERT_EQUAL(true, di->isDiscrete());
  
  m_adapter->processData("TIME|vars2|a=1 b=2 c=3");
  m_adapter->processData("TIME|vars2|c=5");
  m_adapter->processData("TIME|vars2|a=1 c=8");
  
  m_agentTestHelper->m_path = "/sample";
  
  {
    PARSE_XML_RESPONSE;
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:BlockDataSet[1]", "UNAVAILABLE");
    ASSETT_DATA_SET_ENTRY(doc, "BlockDataSet[2]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "BlockDataSet[2]", "b", "2");
    ASSETT_DATA_SET_ENTRY(doc, "BlockDataSet[2]", "c", "3");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:BlockDataSet[2]@count", "3");
    ASSETT_DATA_SET_ENTRY(doc, "BlockDataSet[3]", "c", "5");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:BlockDataSet[3]@count", "1");
    ASSETT_DATA_SET_ENTRY(doc, "BlockDataSet[4]", "a", "1");
    ASSETT_DATA_SET_ENTRY(doc, "BlockDataSet[4]", "c", "8");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:BlockDataSet[4]@count", "2");
  }
  
}

void DataSetTest::testProbe()
{
  m_adapter = m_agent->addAdapter("LinuxCNC", "server", 7878, false);
  CPPUNIT_ASSERT(m_adapter);
  
  m_agentTestHelper->m_path = "/probe";
  
  {
    PARSE_XML_RESPONSE;
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:DataItem[@name='vars']@representation", "DATA_SET");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:DataItem[@name='vars2']@representation", "DATA_SET");
    CPPUNITTEST_ASSERT_XML_PATH_EQUAL(doc, "//m:DataItem[@name='vars2']@discrete", "true");
  }
}
