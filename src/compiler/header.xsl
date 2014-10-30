<xsl:stylesheet version="3.0"
 xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
 xmlns:xs="http://www.w3.org/2001/XMLSchema"
 xmlns:my="my:my"
> 

 <xsl:output method="text" omit-xml-declaration="yes" media-type="text/c++"/>

 <xsl:function name="my:storage_type" as="xs:string">
  <xsl:param name="type" as="xs:string"/>
  <xsl:choose>
   <xsl:when test="$type = 'string'">
    <xsl:text>std::string</xsl:text>
   </xsl:when>
   <xsl:otherwise>
    <xsl:value-of select="concat($type,'*')"/>
   </xsl:otherwise>
  </xsl:choose>
 </xsl:function>

 <xsl:function name="my:return_type" as="xs:string">
  <xsl:param name="type" as="xs:string"/>
  <xsl:choose>
   <xsl:when test="$type = 'string'">
    <xsl:text>const std::string &amp;</xsl:text>
   </xsl:when>
   <xsl:otherwise>
    <xsl:value-of select="concat($type,' *')"/>
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
    <xsl:value-of select="my:storage_type(type)"/>
    <xsl:text> </xsl:text>
    <xsl:value-of select="name"/>
    <xsl:text>;&#xa;</xsl:text>
   </xsl:for-each>

   <!-- Loop over table fields for getters -->
   <xsl:text>&#xa;  public:&#xa;</xsl:text>
   <xsl:for-each select="field">
    <xsl:text>   </xsl:text>
    <xsl:value-of select="my:return_type(type)"/>
    <xsl:text>get_</xsl:text>
    <xsl:value-of select="name"/>
    <xsl:text>() const {return </xsl:text>
    <xsl:value-of select="name"/>
    <xsl:text>;}&#xa;</xsl:text>
   </xsl:for-each>

   <!-- Loop over table fields for setters -->
   <xsl:text>&#xa;</xsl:text>
   <xsl:for-each select="field">
    <xsl:text>   void set_</xsl:text>
    <xsl:value-of select="name"/>
    <xsl:text>(</xsl:text>
    <xsl:value-of select="my:return_type(type)"/>
    <xsl:text>new_</xsl:text>
    <xsl:value-of select="name"/>
    <xsl:text>);&#xa;</xsl:text>
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
