<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"> 
 <xsl:output
  method="text"
  indent="no"
  omit-xml-declaration="yes"
  media-type="text/c++"/>
 <xsl:template match="/database">

  <xsl:text>#ifndef </xsl:text>
  <xsl:value-of select="name"/>
  <xsl:text>_Declared&#xa;</xsl:text>
  <xsl:text>#define </xsl:text>
  <xsl:value-of select="name"/>
  <xsl:text>_Declared&#xa;&#xa;</xsl:text>

  <!-- Loop over tables -->
  <xsl:for-each select="table">
   <xsl:text>class </xsl:text><xsl:value-of select="name"/>
   <xsl:text>&#xa;{&#xa;</xsl:text>

   <!-- Loop over table fields -->
   <xsl:for-each select="field">
    <xsl:text> </xsl:text>
    <xsl:value-of select="type"/>
    <xsl:text> </xsl:text>
    <xsl:value-of select="name"/>
    <xsl:text>;&#xa;</xsl:text>
   </xsl:for-each>

   <xsl:text>};&#xa;&#xa;</xsl:text>
  </xsl:for-each>

  <xsl:text>#endif&#xa;</xsl:text>
 </xsl:template>
</xsl:transform>
