/*
 * SPDX-FileCopyrightText: 2011 Janardhan Reddy <annapareddyjanardhanreddy@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KFILEITEMMODELFILTER_H
#define KFILEITEMMODELFILTER_H

#include "dolphin_export.h"

#include <QStringList>

class KFileItem;
class QRegularExpression;

/**
 * @brief Allows to check whether an item of the KFileItemModel
 *        matches with a set filter-string.
 *
 * Currently the filter is only checked for the KFileItem::text()
 * property of the KFileItem, but this might get extended in
 * future.
 */
class DOLPHIN_EXPORT KFileItemModelFilter
{
public:
    KFileItemModelFilter();
    virtual ~KFileItemModelFilter();

    /**
     * Sets the pattern that is used for a comparison with the item
     * in KFileItemModelFilter::matches(). Per default the pattern
     * defines a sub-string. As soon as the pattern contains at least
     * a '*', '?' or '[' the pattern represents a regular expression.
     */
    void setPattern(const QString &pattern);
    QString pattern() const;

    /**
     * Set the list of mimetypes that are used for comparison with the
     * item in KFileItemModelFilter::matchesMimeType.
     */
    void setMimeTypes(const QStringList &types);
    QStringList mimeTypes() const;

    /**
     * Set the list of mimetypes that are used for comparison and excluded with the
     * item in KFileItemModelFilter::matchesMimeType.
     */
    void setExcludeMimeTypes(const QStringList &types);
    QStringList excludeMimeTypes() const;

    /**
     * Sets whether hidden files should be visible. When false, hidden files
     * will be filtered out unless they match the whitelist patterns.
     */
    void setHiddenFilesShown(bool shown);
    bool hiddenFilesShown() const;

    /**
     * Sets whether the hidden files whitelist is enabled. When enabled,
     * hidden files matching whitelist patterns will be shown even when
     * hidden files are not being shown.
     */
    void setHiddenFilesWhitelistEnabled(bool enabled);
    bool hiddenFilesWhitelistEnabled() const;

    /**
     * Sets the list of patterns for hidden files that should always be shown.
     * Patterns support wildcards (*, ?, [).
     */
    void setHiddenFilesWhitelist(const QStringList &patterns);
    QStringList hiddenFilesWhitelist() const;

    /**
     * @return True if either the pattern or mimetype filters has been set,
     *         or if hidden files filtering is active.
     */
    bool hasSetFilters() const;

    /**
     * @return True if the item matches with the pattern defined by
     *         @ref setPattern() or @ref setMimeTypes
     */
    bool matches(const KFileItem &item) const;

private:
    /**
     * @return True if item matches pattern set by @ref setPattern.
     */
    bool matchesPattern(const KFileItem &item) const;

    /**
     * @return True if item matches mimetypes set by @ref setMimeTypes.
     */
    bool matchesType(const KFileItem &item) const;

    /**
     * @return True if the hidden item matches any of the whitelist patterns.
     */
    bool matchesHiddenWhitelist(const KFileItem &item) const;

    /**
     * Updates the compiled regular expressions for the whitelist patterns.
     */
    void updateHiddenWhitelistRegExps();

    bool m_useRegExp; // If true, m_regExp is used for filtering,
                      // otherwise m_lowerCaseFilter is used.
    QRegularExpression *m_regExp;
    QString m_lowerCasePattern; // Lowercase version of m_filter for
                                // faster comparison in matches().
    QString m_pattern; // Property set by setPattern().
    QStringList m_mimeTypes; // Property set by setMimeTypes()
    QStringList m_excludeMimeTypes; // Property set by setExcludeMimeTypes()

    bool m_hiddenFilesShown; // Whether hidden files should be visible
    bool m_hiddenWhitelistEnabled; // Whether whitelist is active
    QStringList m_hiddenWhitelist; // Patterns for always-visible hidden files
    QList<QRegularExpression> m_hiddenWhitelistRegExps; // Compiled patterns
};
#endif
