<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:pubsubns="http://tempuri.org/PubSub" version="1.0">
  <xsl:output method="text" encoding="UTF-8" media-type="text/plain"/>
  <xsl:template match="/">
    <xsl:for-each xml:space="preserve" select="/soapenv:Envelope/soapenv:Body//statusEvent/item[key='status']/value"><xsl:value-of select="."/>
</xsl:for-each>
  </xsl:template>
</xsl:stylesheet>
