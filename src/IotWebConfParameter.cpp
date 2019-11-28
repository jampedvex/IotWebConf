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

IotWebConfParameter::IotWebConfParameter(
  const char* label, const char* id, char* valueBuffer, int length,
  boolean visible, const char* defaultValue)
{
  this->label = label;
  this->_id = id;
  this->valueBuffer = valueBuffer;
  this->_length = length;
  this->visible = visible;
  this->defaultValue = defaultValue;

  this->errorMessage = NULL;
}

///////////////////////////////////////////////////////////////////////////////

IotWebConfTextParameter::IotWebConfTextParameter(
  const char* label, const char* id, char* valueBuffer, int length,
  boolean visible, const char* defaultValue,
  const char* placeholder,
  const char* customHtml)
  : IotWebConfParameter(label, id, valueBuffer, length, visible, defaultValue)
{
  this->placeholder = placeholder;
  this->customHtml = customHtml;
}

String IotWebConfTextParameter::renderHtml(boolean hasValueFromPost, String valueFromPost)
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
  IotWebConfParameter* current = this;
  newValue.toCharArray(current->valueBuffer, current->getLength());
}

IotWebConfSerializationData IotWebConfTextParameter::serialize()
{
  IotWebConfSerializationData serializationData;
  serializationData.valueBuffer = (byte*)this->valueBuffer;
  serializationData.length = this->getLength();
  return serializationData;
}
IotWebConfSerializationData IotWebConfTextParameter::prepareDeserialization()
{
  IotWebConfSerializationData serializationData;
  serializationData.valueBuffer = (byte*)this->valueBuffer;
  serializationData.length = this->getLength();
  return serializationData;
}
void IotWebConfTextParameter::deserialize(IotWebConfSerializationData serializationData)
{
  IotWebConfParameter* current = this;
  if ((strlen(current->valueBuffer) == 0) && (current->defaultValue != NULL))
  {
    strncpy(current->valueBuffer, current->defaultValue, current->getLength());
#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
    Serial.println(F("Using default "));
#endif
  }
}

void IotWebConfTextParameter::debugToSerial()
{
  IotWebConfParameter* current = this;
  Serial.print("'");
  Serial.print(current->getId());
  Serial.print("' with value: '");
  Serial.print(current->valueBuffer);
  Serial.print("'");
}

///////////////////////////////////////////////////////////////////////////////

IotWebConfNumberParameter::IotWebConfNumberParameter(
  const char* label, const char* id, char* valueBuffer, int length,
  boolean visible, const char* defaultValue,
  const char* placeholder,
  const char* customHtml)
  : IotWebConfTextParameter(label, id, valueBuffer, length, visible, defaultValue,
  placeholder, customHtml)
{
}

String IotWebConfNumberParameter::renderHtml(
  boolean hasValueFromPost, String valueFromPost)
{
  return IotWebConfTextParameter::renderHtml("number", hasValueFromPost, valueFromPost);
}

///////////////////////////////////////////////////////////////////////////////

IotWebConfPasswordParameter::IotWebConfPasswordParameter(
  const char* label, const char* id, char* valueBuffer, int length,
  boolean visible, const char* defaultValue,
  const char* placeholder,
  const char* customHtml)
  : IotWebConfTextParameter(label, id, valueBuffer, length, visible, defaultValue,
  placeholder, customHtml)
{
}

String IotWebConfPasswordParameter::renderHtml(
  boolean hasValueFromPost, String valueFromPost)
{
  return IotWebConfTextParameter::renderHtml("password", true, String(""));
}

void IotWebConfPasswordParameter::debugToSerial()
{
  IotWebConfParameter* current = this;
  Serial.print("'");
  Serial.print(current->getId());
  Serial.print("' with value: ");
#ifdef IOTWEBCONF_DEBUG_PWD_TO_SERIAL
  Serial.print("'");
  Serial.print(current->valueBuffer);
  Serial.print("'");
#else
  Serial.print(F("<hidden>"));
#endif
}

void IotWebConfPasswordParameter::update(String newValue)
{
  IotWebConfParameter* current = this;
//  char temp[IOTWEBCONF_WORD_LEN];
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

IotWebConfSeparator::IotWebConfSeparator()
  : IotWebConfParameter(NULL, NULL, NULL, 0)
{
}

IotWebConfSeparator::IotWebConfSeparator(const char* label)
  : IotWebConfParameter(label, NULL, NULL, 0)
{
}
