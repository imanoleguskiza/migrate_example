<?php
/**
 * @file
 * Script to relate a correct parent menu.
 */

$docs = json_decode(file_get_contents("list.json"));

foreach ($docs as $parent_doc) {
  $json_doc = json_decode(file_get_contents('document-' . $parent_doc . '.json'));
  $json_docs[] = $json_doc;
  // $map[$json_doc->fields->weight->und[0]] = $json_doc->_id;.
  $map[$json_doc->fields->level->und[0]][$json_doc->fields->weight->und[0]] = $json_doc->_id;
}

foreach ($json_docs as $json_doc) {
  if (isset($json_doc->fields->parent->und[0]) && !empty($json_doc->fields->parent->und[0])) {
    $json_doc->fields->parent->und[0] = $map[$json_doc->fields->level->und[0] - 1][$json_doc->fields->parent->und[0]];
    file_put_contents('document-' . $json_doc->_id . '.json', json_encode($json_doc, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE));
  }
}
