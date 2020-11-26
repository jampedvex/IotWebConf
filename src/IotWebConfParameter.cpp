/**
 * IotWebConfParameter.cpp -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf
 *
 * Copyright (C) 2019 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <IotWebConfParameter.h>

IotWebConfParameterGroup::IotWebConfParameterGroup(
  const char* label) :
  IotWebConfConfigItem()
{
  this->label = label;
}

void IotWebConfParameterGroup::addItem(IotWebConfConfigItem* configItem)
{
  if (this->_firstItem == NULL)
  {
    this->_firstItem = configItem;
    return;
  }
  IotWebConfConfigItem* current = this->_firstItem;
  while (current->_nextItem != NULL)
  {
    current = current->_nextItem;
  }
  current->_nextItem = configItem;
}

int IotWebConfParameterGroup::getStorageSize()
{
  int size = 0;
  IotWebConfConfigItem* current = this->_firstItem;
  while (current != NULL)
  {
    size += current->getStorageSize();
    current = current->_nextItem;
  }
  return size;
}
void IotWebConfParameterGroup::storeValue(std::function<void(IotWebConfSerializationData* serializationData)> doStore)
{
  IotWebConfConfigItem* current = this->_firstItem;
  while (current != NULL)
  {
    current->storeValue(doStore);
    current = current->_nextItem;
  }
}
void IotWebConfParameterGroup::loadValue(std::function<void(IotWebConfSerializationData* serializationData)> doLoad)
{
  IotWebConfConfigItem* current = this->_firstItem;
  while (current != NULL)
  {
    current->loadValue(doLoad);
    current = current->_nextItem;
  }
}

void IotWebConfParameterGroup::renderHtml(
  boolean dataArrived, WebServer* server)
{
    if (this->label != NULL)
    {
      String content = "<fieldset>";
      content += "<legend>";
      content += this->label;
      content += "</legend>";
      server->sendContent(content);
    }
    IotWebConfConfigItem* current = this->_firstItem;
    while (current != NULL)
    {
      if (current->visible)
      {
        current->renderHtml(dataArrived, server);
      }
      current = current->_nextItem;
    }
    if (this->label != NULL)
    {
      server->sendContent("</fieldset>");
    }
}
void IotWebConfParameterGroup::update(WebServer* server)
{
  IotWebConfConfigItem* current = this->_firstItem;
  while (current != NULL)
  {
    current->update(server);
    current = current->_nextItem;
  }
}
void IotWebConfParameterGroup::debugTo(Stream* out)
{
  out->print("vv- ");
  out->println(this->label);
  IotWebConfConfigItem* current = this->_firstItem;
  while (current != NULL)
  {
    current->debugTo(out);
    current = current->_nextItem;
  }
  out->println("^^- ");
}

///////////////////////////////////////////////////////////////////////////////

IotWebConfParameter::IotWebConfParameter(
  const char* label, const char* id, char* valueBuffer, int length,
  const char* defaultValue) :
  IotWebConfConfigItem()
{
  this->label = label;
  this->_id = id;
  this->valueBuffer = valueBuffer;
  this->_length = length;
  this->defaultValue = defaultValue;

  this->errorMessage = NULL;
}
int IotWebConfParameter::getStorageSize()
{
  return this->_length;
}
void IotWebConfParameter::storeValue(std::function<void(IotWebConfSerializationData* serializationData)> doStore)
{
  IotWebConfSerializationData serializationData;
  serializationData.length = this->_length;
  serializationData.data = (byte*)this->valueBuffer;
  doStore(&serializationData);
}
void IotWebConfParameter::loadValue(std::function<void(IotWebConfSerializationData* serializationData)> doLoad)
{
  IotWebConfSerializationData serializationData;
  serializationData.length = this->_length;
  serializationData.data = (byte*)this->valueBuffer;
  doLoad(&serializationData);
}
void IotWebConfParameter::update(WebServer* server)
{
  String newValue = server->arg(this->getId());
  this->update(newValue);
}

///////////////////////////////////////////////////////////////////////////////

IotWebConfTextParameter::IotWebConfTextParameter(
  const char* label, const char* id, char* valueBuffer, int length,
  const char* defaultValue,
  const char* placeholder,
  const char* customHtml)
  : IotWebConfParameter(label, id, valueBuffer, length, defaultValue)
{
  this->placeholder = placeholder;
  this->customHtml = customHtml;
}

void IotWebConfTextParameter::renderHtml(
  boolean dataArrived, WebServer* server)
{
  String content = this->renderHtml(dataArrived, server->hasArg(this->getId()), server->arg(this->getId()));
  server->sendContent(content);
}
String IotWebConfTextParameter::renderHtml(boolean dataArrived, boolean hasValueFromPost, String valueFromPost)
{
  return this->renderHtml("text", hasValueFromPost, valueFromPost);
}
String IotWebConfTextParameter::renderHtml(const char* type, boolean hasValueFromPost, String valueFromPost)
{
  IotWebConfTextParameter* current = this;
  char parLength[5];

  String pitem = FPSTR(IOTWEBCONF_HTML_FORM_PARAM);

  pitem.replace("{b}", current->label);
  pitem.replace("{t}", type);
  pitem.replace("{i}", current->getId());
  pitem.replace("{p}", current->placeholder == NULL ? "" : current->placeholder);
  snprintf(parLength, 5, "%d", current->getLength());
  pitem.replace("{l}", parLength);
  if (hasValueFromPost)
  {
    // -- Value from previous submit
    pitem.replace("{v}", valueFromPost);
  }
  else
  {
    // -- Value from config
    pitem.replace("{v}", current->valueBuffer);
  }
  pitem.replace(
      "{c}", current->customHtml == NULL ? "" : current->customHtml);
  pitem.replace(
      "{s}",
      current->errorMessage == NULL ? "" : "de"); // Div style class.
  pitem.replace(
      "{e}",
      current->errorMessage == NULL ? "" : current->errorMessage);

  return pitem;
}

void IotWebConfTextParameter::update(String newValue)
{
  newValue.toCharArray(this->valueBuffer, this->getLength());
}

void IotWebConfTextParameter::debugTo(Stream* out)
{
  IotWebConfParameter* current = this;
  out->print("'");
  out->print(current->getId());
  out->print("' with value: '");
  out->print(current->valueBuffer);
  out->print("'");
}

///////////////////////////////////////////////////////////////////////////////

IotWebConfNumberParameter::IotWebConfNumberParameter(
  const char* label, const char* id, char* valueBuffer, int length,
  const char* defaultValue,
  const char* placeholder,
  const char* customHtml)
  : IotWebConfTextParameter(label, id, valueBuffer, length, defaultValue,
  placeholder, customHtml)
{
}

String IotWebConfNumberParameter::renderHtml(
  boolean dataArrived,
  boolean hasValueFromPost, String valueFromPost)
{
  return IotWebConfTextParameter::renderHtml("number", hasValueFromPost, valueFromPost);
}

///////////////////////////////////////////////////////////////////////////////

IotWebConfPasswordParameter::IotWebConfPasswordParameter(
  const char* label, const char* id, char* valueBuffer, int length,
  const char* defaultValue,
  const char* placeholder,
  const char* customHtml)
  : IotWebConfTextParameter(label, id, valueBuffer, length, defaultValue,
  placeholder, customHtml)
{
}

String IotWebConfPasswordParameter::renderHtml(
  boolean dataArrived,
  boolean hasValueFromPost, String valueFromPost)
{
  return IotWebConfTextParameter::renderHtml("password", true, String(""));
}

void IotWebConfPasswordParameter::debugTo(Stream* out)
{
  IotWebConfParameter* current = this;
  out->print("'");
  out->print(current->getId());
  out->print("' with value: ");
#ifdef IOTWEBCONF_DEBUG_PWD_TO_SERIAL
  out->print("'");
  out->print(current->valueBuffer);
  out->print("'");
#else
  out->print(F("<hidden>"));
#endif
}

void IotWebConfPasswordParameter::update(String newValue)
{
  IotWebConfParameter* current = this;
//  char temp[IOTWEBCONF_PASSWORD_LEN];
  char temp[current->getLength()];
  newValue.toCharArray(temp, current->getLength());
  if (temp[0] != '\0')
  {
    // -- Value was set.
    strncpy(current->valueBuffer, temp, current->getLength());
#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
    Serial.print("Updated ");
#endif
  }
  else
  {
#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
    Serial.println("Was not changed ");
#endif
  }
}

///////////////////////////////////////////////////////////////////////////////

IotWebConfCheckboxParameter::IotWebConfCheckboxParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    boolean defaultValue)
  : IotWebConfTextParameter(label, id, valueBuffer, length, defaultValue ? "selected" : NULL,
  NULL, NULL)
{
}

String IotWebConfCheckboxParameter::renderHtml(
  boolean dataArrived,
  boolean hasValueFromPost, String valueFromPost)
{
  boolean checkSelected = false;
  if (dataArrived)
  {
    if (hasValueFromPost && valueFromPost.equals("selected"))
    {
      checkSelected = true;
    }
  }
  else
  {
    if (this->isChecked())
    {
      checkSelected = true;
    }
  }

  if (checkSelected)
  {
    this->customHtml = IotWebConfCheckboxParameter::_checkedStr;
  }
  else
  {
    this->customHtml = NULL;
  }
  
  
  return IotWebConfTextParameter::renderHtml("checkbox", true, "selected");
}

void IotWebConfCheckboxParameter::update(String newValue)
{
  return IotWebConfTextParameter::update(newValue);
}

///////////////////////////////////////////////////////////////////////////////

IotWebConfOptionsParameter::IotWebConfOptionsParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* optionValues, const char* optionNames, size_t optionCount, size_t nameLength,
    const char* defaultValue)
  : IotWebConfTextParameter(label, id, valueBuffer, length, defaultValue,
  NULL, NULL)
{
  this->_optionValues = optionValues;
  this->_optionNames = optionNames;
  this->_optionCount = optionCount;
  this->_nameLength = nameLength;
}

///////////////////////////////////////////////////////////////////////////////

IotWebConfSelectParameter::IotWebConfSelectParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* optionValues, const char* optionNames, size_t optionCount, size_t nameLength,
    const char* defaultValue)
  : IotWebConfOptionsParameter(label, id, valueBuffer, length, optionValues, optionNames,
  optionCount, nameLength, defaultValue)
{
}

String IotWebConfSelectParameter::renderHtml(
  boolean dataArrived,
  boolean hasValueFromPost, String valueFromPost)
{
  IotWebConfTextParameter* current = this;

  String options = "";

  for (size_t i=0; i<this->_optionCount; i++)
  {
    const char *optionValue = (this->_optionValues + (i*this->getLength()) );
    const char *optionName = (this->_optionNames + (i*this->_nameLength) );
    String oitem = FPSTR(IOTWEBCONF_HTML_FORM_OPTION);
    oitem.replace("{v}", optionValue);
//    if (sizeof(this->_optionNames) > i)
    {
      oitem.replace("{n}", optionName);
    }
//    else
//    {
//      oitem.replace("{n}", "?");
//    }
    if ((hasValueFromPost && (valueFromPost == optionValue)) ||
      (strncmp(current->valueBuffer, optionValue, this->getLength()) == 0))
    {
      // -- Value from previous submit
      oitem.replace("{s}", " selected");
    }
    else
    {
      // -- Value from config
      oitem.replace("{s}", "");
    }

    options += oitem;
  }

  String pitem = FPSTR(IOTWEBCONF_HTML_FORM_SELECT_PARAM);

  pitem.replace("{b}", current->label);
  pitem.replace("{i}", current->getId());
  pitem.replace(
      "{c}", current->customHtml == NULL ? "" : current->customHtml);
  pitem.replace(
      "{s}",
      current->errorMessage == NULL ? "" : "de"); // Div style class.
  pitem.replace(
      "{e}",
      current->errorMessage == NULL ? "" : current->errorMessage);
  pitem.replace("{o}", options);

  return pitem;
}
