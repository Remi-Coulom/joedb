<xsl:stylesheet version="2.0"
 xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
 xmlns:my="http://remi.coulom.free.fr"
> 

 <xsl:output method="text" omit-xml-declaration="yes" media-type="text/c++"/>

 <xsl:function name="my:cpptype">
  <xsl:param name="type"/>
  <xsl:choose>
   <xsl:when test="type = 'string'">
    <xsl:text>std::string</xsl:text>
   </xsl:when>
   <xsl:otherwise>
    <xsl:value-of select="type"/><xsl:text>*</xsl:text>
   </xsl:otherwise>
  </xsl:choose>
 </xsl:function>

 <xsl:template match="/database">

  <xsl:text>#ifndef </xsl:text>
  <xsl:value-of select="name"/>
  <xsl:text>_Declared&#xa;</xsl:text>
  <xsl:text>#define </xsl:text>
  <xsl:value-of select="name"/>
  <xsl:text>_Declared&#xa;&#xa;</xsl:text>

  <xsl:text>class </xsl:text>
  <xsl:value-of select="name"/>
  <xsl:text>&#xa;{&#xa;</xsl:text>

  <!-- Loop over tables -->
  <xsl:for-each select="table">
   <xsl:text> class </xsl:text><xsl:value-of select="name"/>
   <xsl:text>&#xa; {&#xa;</xsl:text>

   <!-- Loop over table fields for private data -->
   <xsl:text>  private:&#xa;</xsl:text>
   <xsl:for-each select="field">
    <xsl:text>   </xsl:text>
    <xsl:value-of select="my:cpptype(type)"/>
    <xsl:text> </xsl:text>
    <xsl:value-of select="name"/>
    <xsl:text>;&#xa;</xsl:text>
   </xsl:for-each>

   <!-- Loop over table fields for getters and setters -->
   <xsl:text>  public:&#xa;</xsl:text>
   <xsl:for-each select="field">
   </xsl:for-each>

   <!-- Table end -->
   <xsl:text> };&#xa;</xsl:text>
   <xsl:if test="position() != last()">
    <xsl:text>&#xa;</xsl:text>
   </xsl:if>
  </xsl:for-each>

  <xsl:text>};&#xa;&#xa;</xsl:text>
  <xsl:text>#endif&#xa;</xsl:text>
 </xsl:template>
</xsl:stylesheet>
