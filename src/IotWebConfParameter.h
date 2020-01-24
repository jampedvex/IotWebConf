/**
 * IotWebConfParameter.h -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf
 *
 * Copyright (C) 2019 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef IotWebConfParameter_h
#define IotWebConfParameter_h

#include <Arduino.h>
#include <IotWebConfSettings.h>

const char IOTWEBCONF_HTML_FORM_PARAM[] PROGMEM =
  "<div class='{s}'><label for='{i}'>{b}</label><input type='{t}' id='{i}' "
  "name='{i}' maxlength={l} placeholder='{p}' value='{v}' {c}/>"
  "<div class='em'>{e}</div></div>";

typedef struct IotWebConfSerializationData
{
  byte* valueBuffer;
  int length;
} IotWebConfSerializationData;


/**
 *   IotWebConfParameters is a configuration item of the config portal.
 *   The parameter will have its input field on the configuration page,
 *   and the provided value will be saved to the EEPROM.
 */
class IotWebConfParameter
{
public:
  /**
   * Create a parameter for the config portal.
   *
   *   @label - Displayable label at the config portal.
   *   @id - Identifier used for HTTP queries and as configuration key. Must not
   * contain spaces nor other special characters.
   *   @valueBuffer - Configuration value will be loaded to this buffer from the
   * EEPROM.
   *   @length - The buffer should have a length provided here.
   *   @defaultValue (optional) - Value should be pre-filled if none was
   * specified before.
   */
  IotWebConfParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    boolean visible = true, const char* defaultValue = NULL);

  virtual String renderHtml(boolean hasValueFromPost, String valueFromPost);
  virtual void update(String newValue);
  virtual IotWebConfSerializationData serialize();
  virtual IotWebConfSerializationData prepareDeserialization();
  virtual void deserialize(IotWebConfSerializationData serializationData);
  virtual void debugToSerial();

  const char* label;
  char* valueBuffer;
  boolean visible;
  const char* defaultValue;
  const char* errorMessage;

  const char* getId() { return this->_id; }
  int getLength() { return this->_length; }

protected:
  IotWebConfParameter() {};

private:
  const char* _id = 0;
  int _length;
  IotWebConfParameter* _nextParameter = NULL;
  friend class IotWebConf; // Allow IotWebConf to access private members.
};

///////////////////////////////////////////////////////////////////////////////

/**
 *   IotWebConfTexParameters is to store text based parameters.
 */
class IotWebConfTextParameter : public IotWebConfParameter
{
public:
  /**
   * Create a parameter for the config portal.
   *
   *   @label - Displayable label at the config portal.
   *   @id - Identifier used for HTTP queries and as configuration key. Must not
   * contain spaces nor other special characters.
   *   @valueBuffer - Configuration value will be loaded to this buffer from the
   * EEPROM.
   *   @length - The buffer should have a length provided here.
   *   @type (optional, default="text") - The type of the html input field.
   *       The type="password" has a special handling, as the value will be
   * overwritten in the EEPROM only if value was provided on the config portal.
   * Because of this logic, "password" type field with length more then
   * IOTWEBCONF_WORD_LEN characters are not supported.
   *   @placeholder (optional) - Text appear in an empty input box.
   *   @customHtml (optional) - The text of this parameter will be added into
   * the HTML INPUT field.
   */
  IotWebConfTextParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    boolean visible = true, const char* defaultValue = NULL,
    const char* placeholder = NULL,
    const char* customHtml = NULL);

  const char* placeholder;
  const char* customHtml;

  virtual String renderHtml(boolean hasValueFromPost, String valueFromPost) override;
  virtual void update(String newValue) override;
  virtual IotWebConfSerializationData serialize() override;
  virtual IotWebConfSerializationData prepareDeserialization() override;
  virtual void deserialize(IotWebConfSerializationData serializationData) override;
  virtual void debugToSerial() override;

protected:
  virtual String renderHtml(const char* type, boolean hasValueFromPost, String valueFromPost);
  IotWebConfTextParameter() {};

private:
  friend class IotWebConf;
};

///////////////////////////////////////////////////////////////////////////////

class IotWebConfPasswordParameter : public IotWebConfTextParameter
{
public:
  IotWebConfPasswordParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    boolean visible = true, const char* defaultValue = NULL,
    const char* placeholder = NULL,
    const char* customHtml = NULL);

  virtual String renderHtml(boolean hasValueFromPost, String valueFromPost) override;
  virtual void update(String newValue) override;
  virtual void debugToSerial() override;

private:
  IotWebConfPasswordParameter() {};
  friend class IotWebConf;
};

///////////////////////////////////////////////////////////////////////////////

class IotWebConfNumberParameter : public IotWebConfTextParameter
{
public:
  IotWebConfNumberParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    boolean visible = true, const char* defaultValue = NULL,
    const char* placeholder = NULL,
    const char* customHtml = NULL);

  virtual String renderHtml(boolean hasValueFromPost, String valueFromPost) override;

private:
  IotWebConfNumberParameter() {};
  friend class IotWebConf;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * A separator for separating field sets.
 * NOTE: Separator will be replaced by a grouping ability of the parameters.
 */
class IotWebConfSeparator : public IotWebConfParameter
{
public:
  IotWebConfSeparator();

  /**
   * Create a seperator with a label (legend tag)
   */
  IotWebConfSeparator(const char* label);

  virtual String renderHtml(boolean hasValueFromPost, String valueFromPost) override;
  virtual void update(String newValue) override;
  virtual IotWebConfSerializationData serialize() override;
  virtual IotWebConfSerializationData prepareDeserialization() override;
  virtual void deserialize(IotWebConfSerializationData serializationData) override;
  virtual void debugToSerial() override;
};

#endif
