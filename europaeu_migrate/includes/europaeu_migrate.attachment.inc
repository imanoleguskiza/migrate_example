<?php

/**
 * @file
 * Contains EuropaEuMigrateTopicAttachment class.
 */

/**
 * Class EuropaEuMigrateTopicAttachment .
 */
class EuropaEuMigrateTopicAttachment extends EuropaEuAbstractMigration {

  /**
   * {@inheritdoc}
   */
  public function __construct($arguments) {
    parent::__construct('bottom_pdf', $arguments);

    $this->setMap(new MigrateSQLMap(
      $this->getMachineName(),
      $this->getSourceKey(),
      MigrateDestinationNode::getKeySchema()
    ));
    $this->setDestination(new MigrateDestinationNode('topic_attachment', array('text_format' => 'full_html')));

    // Entity translation requires that both title fields are mapped.
    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('title_field', 'title');
    $this->addFieldMapping('body', 'pdf_text');
    $this->addFieldMapping('field_topic_attachment_date', 'date');

    $this->addFieldMapping('field_topic_attachment_image', 'pdf_preview');

    $this->addFieldMapping('field_topic_attachment_file', 'pdf_url');
    $this->addFieldMapping('field_topic_attachment_file:description', 'pdf_alt_text');

    // Mapping default language is necessary for correct translation handling.
    $this->addFieldMapping('language', 'default_language');

    $this->addFieldMapping('promote')->defaultValue(FALSE);
    $this->addFieldMapping('status')->defaultValue(NODE_PUBLISHED);
  }

}
