<?php

/*
 * contains methods to search solr and display results.  depends on Apache_Solr_Php client.
 */


/**
 * Extention of IslandoraSolrResults
 * This overrides the printResults function to provide customized display.
 */
module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrResults');

class ADRCOSPLSolrResults extends IslandoraSolrResults {

  static $facetSeparator = '~'; //used to separate facets in url
  static $slashReplacement = '~slsh~'; // a pattern used to replace / in url's the slash breaks drupal clean url's
  public static $SEARCH_CLASS_ADVANCED_SEARCH_NUMBER_FIELDS = 5;

  function IslandoraSolrResults() {
    module_load_include('php', 'islandora_solr_search', 'Solr/Service');
  }

  /**
   *
   * @param $results
   *   A solr resultset object.
   *
   * @return
   *   html output for the resultset. Note: we currently create this
   *   output manually, should refactor to use drupal forms api.
   */
  function printResults($results) {

    global $base_url;
    if (empty($results)) {
      return "no results";
    }

    $moduleRoot = drupal_get_path('module', 'islandora_co_solr_config');
    drupal_add_css($moduleRoot . '/css/adr.css');
    drupal_add_css($moduleRoot . '/css/jquery.lightbox-0.5.css');
    drupal_add_js($moduleRoot . '/js/jquery.lightbox-0.5.modified.js');
    drupal_add_js($moduleRoot . '/js/uofm.js');

    drupal_add_js("$(document).ready(function () {  $('.results a.zoomIcon').lightBox( { txtImage: 'Result', imageLoading: '$base_url/$moduleRoot/images/lightbox-icon-loading.gif',  imageBtnClose: '$base_url/$moduleRoot/images/lightbox-btn-close.gif',  imageBtnPrev: '$base_url/$moduleRoot/images/lightbox-btn-prev.gif', imageBtnNext: '$base_url/$moduleRoot/images/lightbox-btn-next.gif'  });  });", 'inline');
    $output = '';
    $output.='<div class="results"/><table border="0" cellpadding="10" cellspacing="0" width="100%">';
    $arg = join('/', arg());

    $rawResponse = $results->getRawResponse();
    $responseArray = json_decode($rawResponse, true);
    $highlightResults = $responseArray['highlighting'];
    $keys = array('');
    $snippetArray = islandora_build_substitution_list(variable_get("islandora_solr_snippet_field", ""));
    if (is_array($snippetArray)) {
      $keys = array_keys($snippetArray);
    }
    $highlight_field = $keys[0];
    $highlight_label = $snippetArray[$highlight_field];

    foreach ($responseArray['response']['docs'] as $doc) {
      $output .= $this->build_list($doc, $highlightResults, $highlight_field);
    }
    $output.='</table></div>';

    return $output;
  }

  function build_list($doc, $highlightResults, $highlight_field) {
    $pid = $doc['PID'];
    $highlight_text = "";
    if (is_array($highlightResults[$pid][$highlight_field])) {
      $highlight_text = implode(' ', $highlightResults[$pid][$highlight_field]);
    }

    $title = $doc['Title'];
    if (is_array($doc['Title'])) {
      $title = (string) $title[0];
    }
    $output .= '<tr><td><div class="thumbnail">';
    $img = "<img src='$base_url/fedora/repository/$pid/TN'/>";
    $output .= l($img, "fedora/repository/$pid", array('attributes' => array('title' => $title), 'html' => 'true'));
    $output .= '<td><div class="metadata"><a href="' . $base_url . '/fedora/repository/' . $pid . '">' . $title . '</a><ul>';

    if (!empty($highlight_text)) {
      $output .= '<li class="fulltext">';
      if ($highlight_label) {
        $output.="<b>$highlight_label</b>&nbsp;";
      }
      $output .= '...';
      $output .= $highlight_text;
      $output .= '...';
      $output .= '</li>';
    }

    
    if (!empty($doc['COSPL_CC'])) {
      $output.='<li class="gray"><b>Creators:</b>&nbsp;';
      if (is_array($doc['COSPL_CC'])) {
        foreach ($doc['COSPL_CC'] as $cosplcc) {
          $output .= "$cosplcc; ";
        }
      }
      else {
        $output .=$doc['COSPL_CC'];
      }
      $output .= '</li>';
    }

 if (!empty($doc['COSPL_Agency'])) {
      $output.='<li class="gray"><b>Agency:</b>&nbsp;';
      if (is_array($doc['COSPL_Agency'])) {
        foreach ($doc['COSPL_Agency'] as $cosplagency) {
          $output .= "$cosplagency; ";
        }
      }
      else {
        $output .=$doc['COSPL_Agency'];
      }
      $output .= '</li>';
    }

      if (!empty($doc['Date_Issued'])) {
      $output.='<li class="gray"><b>Publication Date:</b>&nbsp;';
      if (is_array($doc['Date_Issued'])) {
        foreach ($doc['Date_Issued'] as $dateIssued) {
          $output .= "$dateIssued";
        }
      }
      else {
        $output .= $ $doc['Date_Issued'];
      }
      $output .= '</li>';
    }

    $output.='</ul></div></td></tr>';
    return $output;
  }

}