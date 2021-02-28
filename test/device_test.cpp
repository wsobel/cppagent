//
// Copyright Copyright 2009-2021, AMT – The Association For Manufacturing Technology (“AMT”)
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

// Ensure that gtest is the first header otherwise Windows raises an error
#include <gtest/gtest.h>
// Keep this comment to keep gtest.h above. (clang-format off/on is not working here!)

#include "device_model/device.hpp"
using namespace std;
using namespace mtconnect;
using namespace entity;
using namespace device_model;
using namespace data_item;

class DeviceTest : public testing::Test
{
 protected:
  void SetUp() override
  {
    std::map<string, string> attributes1;
    attributes1["id"] = "1";
    attributes1["name"] = "DeviceTest1";
    attributes1["uuid"] = "UnivUniqId1";
    attributes1["iso841Class"] = "4";
    m_devA = new Device(attributes1);

    std::map<string, string> attributes2;
    attributes2["id"] = "3";
    attributes2["name"] = "DeviceTest2";
    attributes2["uuid"] = "UnivUniqId2";
    attributes2["sampleRate"] = "123.4";
    attributes2["iso841Class"] = "6";
    m_devB = new Device(attributes2);
  }

  void TearDown() override
  {
    delete m_devA;
    m_devA = nullptr;
    delete m_devB;
    m_devB = nullptr;
  }

  Device *m_devA{nullptr}, *m_devB{nullptr};
};

TEST_F(DeviceTest, Getters)
{
  ASSERT_EQ((string) "Device", m_devA->getClass());
  ASSERT_EQ((string) "1", m_devA->getId());
  ASSERT_EQ((string) "DeviceTest1", m_devA->getName());
  ASSERT_EQ((string) "UnivUniqId1", m_devA->getUuid());

  ASSERT_EQ((string) "Device", m_devB->getClass());
  ASSERT_EQ((string) "3", m_devB->getId());
  ASSERT_EQ((string) "DeviceTest2", m_devB->getName());
  ASSERT_EQ((string) "UnivUniqId2", m_devB->getUuid());
}

TEST_F(DeviceTest, GetAttributes)
{
  const auto &attributes1 = m_devA->getAttributes();

  ASSERT_EQ((string) "1", attributes1.at("id"));
  ASSERT_EQ((string) "DeviceTest1", attributes1.at("name"));
  ASSERT_EQ((string) "UnivUniqId1", attributes1.at("uuid"));
  ASSERT_TRUE(attributes1.find("sampleRate") == attributes1.end());
  ASSERT_EQ((string) "4", attributes1.at("iso841Class"));

  const auto &attributes2 = m_devB->getAttributes();

  ASSERT_EQ((string) "3", attributes2.at("id"));
  ASSERT_EQ((string) "DeviceTest2", attributes2.at("name"));
  ASSERT_EQ((string) "UnivUniqId2", attributes2.at("uuid"));
  ASSERT_EQ((string) "123.400001525879", attributes2.at("sampleInterval"));
  ASSERT_EQ((string) "6", attributes2.at("iso841Class"));
}

TEST_F(DeviceTest, Description)
{
  map<string, string> attributes;
  attributes["manufacturer"] = "MANUFACTURER";
  attributes["serialNumber"] = "SERIAL_NUMBER";

  m_devA->addDescription((string) "Machine 1", attributes);
  auto description1 = m_devA->getDescription();

  ASSERT_EQ((string) "MANUFACTURER", description1["manufacturer"]);
  ASSERT_EQ((string) "SERIAL_NUMBER", description1["serialNumber"]);
  ASSERT_TRUE(description1["station"].empty());

  ASSERT_EQ((string) "Machine 1", m_devA->getDescriptionBody());

  attributes["station"] = "STATION";
  m_devB->addDescription((string) "Machine 2", attributes);
  auto description2 = m_devB->getDescription();

  ASSERT_EQ((string) "MANUFACTURER", description2["manufacturer"]);
  ASSERT_EQ((string) "SERIAL_NUMBER", description2["serialNumber"]);
  ASSERT_EQ((string) "STATION", description2["station"]);

  ASSERT_EQ((string) "Machine 2", m_devB->getDescriptionBody());
}

TEST_F(DeviceTest, Relationships)
{
  // Test get/set parents
  map<string, string> dummy;
  auto *linear = new mtconnect::Component("Linear", dummy);

  auto devPointer = dynamic_cast<mtconnect::Component *>(m_devA);
  ASSERT_TRUE(devPointer);

  m_devA->addChild(linear);
  ASSERT_TRUE(devPointer == linear->getParent());

  auto *controller = new mtconnect::Component("Controller", dummy);
  m_devA->addChild(controller);
  ASSERT_TRUE(devPointer == controller->getParent());

  // Test get device
  ASSERT_TRUE(m_devA == m_devA->getDevice());
  ASSERT_TRUE(m_devA == linear->getDevice());
  ASSERT_TRUE(m_devA == controller->getDevice());

  // Test add/get children
  ASSERT_FALSE(m_devA->getChildren().empty());

  auto *axes = new mtconnect::Component("Axes", dummy),
       *thermostat = new mtconnect::Component("Thermostat", dummy);
  m_devA->addChild(axes);
  m_devA->addChild(thermostat);

  ASSERT_EQ((size_t)4, m_devA->getChildren().size());
  auto it = m_devA->getChildren().begin();
  ASSERT_EQ(linear, *it); it++;
  ASSERT_EQ(controller, *it); it++;
  ASSERT_EQ(axes, *it); it++;
  ASSERT_EQ(thermostat, *it); it++;
  ASSERT_EQ(m_devA->getChildren().end(), it);
}

TEST_F(DeviceTest, DataItems)
{
  ASSERT_TRUE(m_devA->getDataItems().empty());

  ErrorList errors;
  auto data1 = DataItem::make({{"id", "a"s}, {"type", "A"s}, {"category", "EVENT"s}}, errors);
  ASSERT_TRUE(errors.empty());
  auto data2 = DataItem::make({{"id", "b"s}, {"type", "A"s}, {"category", "EVENT"s}}, errors);
  ASSERT_TRUE(errors.empty());
  m_devA->addDataItem(data1);
  m_devA->addDataItem(data2);

  ASSERT_EQ((size_t)2, m_devA->getDataItems().size());
  ASSERT_TRUE(data1 == m_devA->getDataItems().front());
  ASSERT_TRUE(data2 == m_devA->getDataItems().back());
}

TEST_F(DeviceTest, DeviceDataItem)
{
  ASSERT_TRUE(m_devA->getDeviceDataItems().empty());
  ASSERT_TRUE(!m_devA->getDeviceDataItem("DataItemTest1"));
  ASSERT_TRUE(!m_devA->getDeviceDataItem("DataItemTest2"));

  
  ErrorList errors;
  auto data1 = DataItem::make({{"id", "DataItemTest1"s}, {"type", "A"s}, {"category", "EVENT"s}}, errors);
  ASSERT_TRUE(errors.empty());
  auto data2 = DataItem::make({{"id", "DataItemTest2"s}, {"type", "A"s}, {"category", "EVENT"s}}, errors);
  ASSERT_TRUE(errors.empty());

  m_devA->addDataItem(data1);
  m_devA->addDataItem(data2);

  ASSERT_EQ((size_t)2, m_devA->getDeviceDataItems().size());
  ASSERT_TRUE(data1 == m_devA->getDeviceDataItem("DataItemTest1"));
  ASSERT_TRUE(data2 == m_devA->getDeviceDataItem("DataItemTest2"));
}

TEST_F(DeviceTest, GetDataItem)
{
  ErrorList errors;
  Properties sp1{{"VALUE", "by_source"s}};
  auto source1 = Source::getFactory()->make("Source", sp1, errors);
  auto data1 = DataItem::make({{"id", "by_id"s}, {"type", "A"s}, {"category", "EVENT"s}, {"name", "by_name"s}, {"Source", source1}}, errors);
  ASSERT_TRUE(errors.empty());
  m_devA->addDataItem(data1);

  Properties sp2{{"VALUE", "by_source2"s}};
  auto source2 = Source::getFactory()->make("Source", sp2, errors);
  auto data2 = DataItem::make({{"id", "by_id2"s}, {"type", "A"s}, {"name", "by_name2"s}, {"category", "EVENT"s}, {"Source", source2}}, errors);
  ASSERT_TRUE(errors.empty());
  m_devA->addDataItem(data2);

  Properties sp{{"VALUE", "by_source3"s}};
  auto source = Source::getFactory()->make("Source", sp, errors);
  Properties p3({{"id", "by_id3"s}, {"type", "A"s}, {"name", "by_name3"s}, {"category", "EVENT"s}, {"Source", source}});
  auto data3 = DataItem::make(p3, errors);
  ASSERT_TRUE(errors.empty());

  m_devA->addDataItem(data3);

  ASSERT_TRUE(data1 == m_devA->getDeviceDataItem("by_id"));
  ASSERT_TRUE(m_devA->getDeviceDataItem("by_name"));
  ASSERT_TRUE(m_devA->getDeviceDataItem("by_source"));

  ASSERT_TRUE(data2 == m_devA->getDeviceDataItem("by_id2"));
  ASSERT_TRUE(data2 == m_devA->getDeviceDataItem("by_name2"));
  ASSERT_TRUE(m_devA->getDeviceDataItem("by_source2"));

  ASSERT_TRUE(data3 == m_devA->getDeviceDataItem("by_id3"));
  ASSERT_TRUE(data3 == m_devA->getDeviceDataItem("by_name3"));
  ASSERT_TRUE(data3 == m_devA->getDeviceDataItem("by_source3"));
}
