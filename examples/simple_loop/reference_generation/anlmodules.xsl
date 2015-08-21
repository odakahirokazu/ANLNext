<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">

  <xsl:output method="html" indent="yes" doctype-public="-//W3C//DTD XHTML 1.1//EN" doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"/>
  
  <xsl:template match="/">
    <html xmlns="http://www.w3.org/1999/xhtml">
      <xsl:copy-of select="anl_doc/head" />
      <body>
        <xsl:for-each select="anl_doc/body_header/div">
	        <xsl:copy-of select="." />
        </xsl:for-each>
	      <h1><xsl:value-of select="anl_doc/head/title" /></h1>
	      <xsl:apply-templates select="anl_doc/package" mode="list" />
	      <xsl:apply-templates select="anl_doc/package" mode="detail" />	      
      </body>
    </html>
  </xsl:template>

  <xsl:template match="package" mode="list">
    <h2>List of modules</h2>
    <xsl:apply-templates select="module_info" mode="list" />
  </xsl:template>

  <xsl:template match="package" mode="detail">
    <xsl:apply-templates select="module_info" mode="detail" />
  </xsl:template>

  <xsl:template match="module_info" mode="list">
    <xsl:variable name="mod" select="document(@href)" />
    <xsl:apply-templates select="$mod//namespace" mode="h3" />
    <ul>
      <xsl:apply-templates select="$mod//module" mode="list" />
    </ul>
  </xsl:template>

  <xsl:template match="module_info" mode="detail">
    <xsl:variable name="mod" select="document(@href)" />
    <xsl:apply-templates select="$mod//namespace" mode="h2" />
    <xsl:apply-templates select="$mod//module" mode="detail" />
  </xsl:template>

  <xsl:template match="namespace" mode="h2">
    <h2><xsl:value-of select="." /></h2>
  </xsl:template>

  <xsl:template match="namespace" mode="h3">
    <h3><xsl:value-of select="." /></h3>
  </xsl:template>

  <xsl:template match="module" mode="list" >
    <xsl:variable name="module_name" select="name" />
    <li>
      <a>
	      <xsl:attribute name="href">#<xsl:value-of select="$module_name" /></xsl:attribute>
	      <xsl:value-of select="$module_name" />
      </a>
    </li>
  </xsl:template>

  <xsl:template match="module" mode="detail" >
    <xsl:apply-templates />
    <!--
        <xsl:apply-templates select="name" />
        <xsl:apply-templates select="super" />
        <xsl:apply-templates select="text" />
        <xsl:apply-templates select="parameters" />
    -->
  </xsl:template>

  <xsl:template match="name">
    <xsl:variable name="name_id" select="." />
    <h3>
      <xsl:attribute name="id"><xsl:value-of select="$name_id" /></xsl:attribute>
      <xsl:value-of select="." />
    </h3>
  </xsl:template>

  <xsl:template match="version">
    <p>Version: <xsl:value-of select="." /></p>
  </xsl:template>

  <xsl:template match="super">
    <xsl:variable name="super_id" select="." />
    <p>Derived from:
    <span style="font-weight: bold;">
	    <a>
	      <xsl:attribute name="href">#<xsl:value-of select="$super_id" /></xsl:attribute>
	      <xsl:value-of select="$super_id" />
	    </a>
    </span>
    </p>
  </xsl:template>

  <xsl:template match="sa">
    <xsl:variable name="module_id" select="." />
    <p>See also: 
    <span style="font-weight: bold;">
	    <a>
	      <xsl:attribute name="href">#<xsl:value-of select="$module_id" /></xsl:attribute>
	      <xsl:value-of select="$module_id" />	
	    </a>
    </span>
    </p>
  </xsl:template>

  <xsl:template match="text">
    <p><xsl:value-of select="." /></p>
  </xsl:template>

  <xsl:template match="parameters">
    <table frame="border" rules="all">
      <caption class="mod_param_cap">Module parameters</caption>
      <colgroup><col width="12%" /><col width="8%" /><col width="8%" /><col width="6%" /><col width="10%" /><col width="56%" /></colgroup>
      <thead>
	      <tr>
	        <th>Name</th>
	        <th>Container</th>
          <th>Type</th>
	        <th>Unit</th>
	        <th>Default value</th>
	        <th>Description</th>
	      </tr>
      </thead>
      <tbody>
	      <xsl:apply-templates select="param" />
      </tbody>
    </table>
  </xsl:template>

  <xsl:template match="param">
    <tr>
      <xsl:if test="string-length(@container_type)&gt;0">
	      <xsl:attribute name="class">container_type_<xsl:value-of select="@container_type" /></xsl:attribute>
      </xsl:if>
      <td><xsl:value-of select="name" /></td>
      <td><xsl:if test="string-length(@container_type)&gt;0"><xsl:value-of select="@container_type" /></xsl:if></td>
      <td><xsl:value-of select="type" /></td>
      <td><xsl:value-of select="unit" /></td>
      <td><xsl:value-of select="default_value" /></td>
      <td><xsl:value-of select="description" /></td>
    </tr>
  </xsl:template>
  
</xsl:stylesheet>
